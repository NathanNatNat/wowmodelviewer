/*----------------------------------------------------------------------*\
| This file is part of WoW Model Viewer                                  |
|                                                                        |
| WoW Model Viewer is free software: you can redistribute it and/or      |
| modify it under the terms of the GNU General Public License as         |
| published by the Free Software Foundation, either version 3 of the     |
| License, or (at your option) any later version.                        |
|                                                                        |
| WoW Model Viewer is distributed in the hope that it will be useful,    |
| but WITHOUT ANY WARRANTY; without even the implied warranty of         |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          |
| GNU General Public License for more details.                           |
|                                                                        |
| You should have received a copy of the GNU General Public License      |
| along with WoW Model Viewer.                                           |
| If not, see <http://www.gnu.org/licenses/>.                            |
\*----------------------------------------------------------------------*/

/*
 * WoWItem.cpp
 *
 *  Created on: 5 feb. 2015
 *      Copyright: 2015 , WoW Model Viewer (http://wowmodelviewer.net)
 */

#include "WoWItem.h"

#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>


#include "Attachment.h"
#include "database.h" // items
#include "Game.h"
#include "RaceInfos.h"
#include "wow_enums.h"
#include "WoWModel.h"

#include "logger/Logger.h"

map<CharSlots, int> WoWItem::SLOT_LAYERS = { { CS_SHIRT, 10 }, { CS_HEAD, 11 }, { CS_SHOULDER, 13 },
                                             { CS_PANTS, 10 }, { CS_BOOTS, 11 }, { CS_CHEST, 13 },
                                             { CS_TABARD, 17 }, { CS_BELT, 18 }, { CS_BRACERS, 19 },
                                             { CS_GLOVES, 20 }, { CS_HAND_RIGHT, 21 }, { CS_HAND_LEFT, 22 },
                                             { CS_CAPE, 23 }, { CS_QUIVER, 24 } };


WoWItem::WoWItem(CharSlots slot)
  : m_charModel(nullptr), m_id(-1), m_displayId(-1),
  m_quality(0), m_level(0), m_type(0),
  m_nbLevels(0), m_slot(slot), m_mergedModel(0), display_flags(0)
{
  setName("---- None ----");
}

void WoWItem::setId(int id)
{
  if (id != m_id)
  {
    m_id = id;

    if (m_id == 0)
    {
      unload();
      // reset name and quality
      setName("---- None ----");
      m_quality = 0;
      m_type = 0;

      if (m_slot == CS_HAND_RIGHT)
        m_charModel->charModelDetails.closeRHand = false;

      if (m_slot == CS_HAND_LEFT)
        m_charModel->charModelDetails.closeLHand = false;

      return;
    }

    QString query = QString("SELECT ItemLevel, ItemAppearanceID, ItemAppearanceModifierID FROM ItemModifiedAppearance WHERE ItemID = %1").arg(id);
    sqlResult itemlevels = GAMEDATABASE.sqlQuery(query);

    if (itemlevels.valid && !itemlevels.values.empty())
    {
      m_nbLevels = 0;
      m_level = 0;
      m_levelDisplayMap.clear();
      for (unsigned int i = 0; i < itemlevels.values.size(); i++)
      {
        int curid = itemlevels.values[i][1].toInt();
        m_modifierIdDisplayMap[itemlevels.values[i][2].toInt()] = curid;
        // if display id is null (case when item's look doesn't change with level)
        if (curid == 0)
          continue;

        //check if display id already in the map (do not duplicate when look is the same)
        bool found = false;
        for (std::map<int, int>::iterator it = m_levelDisplayMap.begin(); it != m_levelDisplayMap.end(); ++it)
        {
          if (it->second == curid)
          {
            found = true;
            break;
          }
        }

        if (!found)
        {
          m_levelDisplayMap[m_nbLevels] = curid;
          m_nbLevels++;
        }
      }
    }

    query = QString("SELECT ItemDisplayInfoID FROM ItemAppearance WHERE ID = %1")
      .arg(m_levelDisplayMap[m_level]);

    sqlResult iteminfos = GAMEDATABASE.sqlQuery(query);

    if (iteminfos.valid && !iteminfos.values.empty())
      m_displayId = iteminfos.values[0][0].toInt();

    ItemRecord itemRcd = items.getById(id);
    setName(itemRcd.name);
    m_quality = itemRcd.quality;
    m_type = itemRcd.type;
    load();
  }
}

void WoWItem::setDisplayId(int id)
{
  if (m_displayId != id)
  {
    m_id = -1;
    m_displayId = id; // to update from database;
    setName("NPC Item");
    load();
  }
}

void WoWItem::setLevel(int level)
{
  if ((m_nbLevels > 1) && (m_level != level))
  {
    m_level = level;

    QString query = QString("SELECT ItemDisplayInfoID FROM ItemAppearance WHERE ID = %1")
      .arg(m_levelDisplayMap[m_level]);

    sqlResult iteminfos = GAMEDATABASE.sqlQuery(query);

    if (iteminfos.valid && !iteminfos.values.empty())
      m_displayId = iteminfos.values[0][0].toInt();

    ItemRecord itemRcd = items.getById(m_id);
    setName(itemRcd.name);
    m_quality = itemRcd.quality;
    m_type = itemRcd.type;
    load();
  }
}

void WoWItem::setModifierId(int id)
{
  auto it = m_modifierIdDisplayMap.find(id);
  if (it != m_modifierIdDisplayMap.end())
  {
    QString query = QString("SELECT ItemDisplayInfoID FROM ItemAppearance WHERE ID = %1")
      .arg(it->second);

    sqlResult iteminfos = GAMEDATABASE.sqlQuery(query);

    if (iteminfos.valid && !iteminfos.values.empty())
      m_displayId = iteminfos.values[0][0].toInt();

    ItemRecord itemRcd = items.getById(m_id);
    setName(itemRcd.name);
    m_quality = itemRcd.quality;
    m_type = itemRcd.type;
    load();
  }
}

void WoWItem::onParentSet(Component * parent)
{
  m_charModel = dynamic_cast<WoWModel *>(parent);
}

void WoWItem::unload()
{
  // delete models and clear map
  for (std::map<POSITION_SLOTS, WoWModel *>::iterator it = m_itemModels.begin(),
       itEnd = m_itemModels.end();
       it != itEnd;
  ++it)
  {
    delete it->second;
  }
  m_itemModels.clear();

  // release textures and clear map
  for (std::map<CharRegions, GameFile *>::iterator it = m_itemTextures.begin(),
       itEnd = m_itemTextures.end();
       it != itEnd;
  ++it)
  {
    TEXTUREMANAGER.delbyname(it->second->fullname());
  }
  m_itemTextures.clear();

  // clear map
  m_itemGeosets.clear();

  // remove any existing attachement
  if (m_charModel->attachment)
    m_charModel->attachment->delSlot(m_slot);

  // unload any merged model
  if (m_mergedModel != 0)
  {
    // TODO : unmerge trigs refreshMerging that trigs refresh... so m_mergedModel must be null...
    // need to find a better way to solve this
    WoWModel * m = m_mergedModel;
    m_mergedModel = 0;
    m_charModel->unmergeModel(m);
    delete m_mergedModel;
  }
}

void WoWItem::load()
{
  unload();

  if (!m_charModel) // no parent => give up
    return;

  if (m_id == 0 || m_displayId == 0) // no equipment, just return
    return;

  RaceInfos charInfos;
  RaceInfos::getCurrent(m_charModel, charInfos);
  sqlResult iteminfos;

  // query geosets infos
  if (!queryItemInfo(QString("SELECT GeoSetGroup1, GeoSetGroup2, GeoSetGroup3, GeoSetGroup4, GeoSetGroup5, GeoSetGroup6, "
                             "AttachmentGeoSetGroup1, AttachmentGeoSetGroup2, AttachmentGeoSetGroup3, "
                             "AttachmentGeoSetGroup4, AttachmentGeoSetGroup5, AttachmentGeoSetGroup6, DisplayFlags "
                             "FROM ItemDisplayInfo WHERE ItemDisplayInfo.ID = %1").arg(m_displayId), 
                     iteminfos))
    return;

  int geosetGroup[6] = { iteminfos.values[0][0].toInt(), iteminfos.values[0][1].toInt() ,
                         iteminfos.values[0][2].toInt(), iteminfos.values[0][3].toInt() ,
                         iteminfos.values[0][4].toInt(), iteminfos.values[0][5].toInt() };

  int attachmentGeosetGroup[6] =
                       { iteminfos.values[0][6].toInt(), iteminfos.values[0][7].toInt() ,
                         iteminfos.values[0][8].toInt(), iteminfos.values[0][9].toInt() ,
                         iteminfos.values[0][10].toInt(), iteminfos.values[0][11].toInt() };
  
  display_flags = iteminfos.values[0][12].toInt();
                         
  // query models
  int model[2] = { getCustomModelId(0), getCustomModelId(1) };

  // query textures
  int texture[2] = { getCustomTextureId(0), getCustomTextureId(1) };

  // query textures from ItemDisplayInfoMaterialRes (if relevant)
  sqlResult texinfos = GAMEDATABASE.sqlQuery(QString("SELECT * FROM ItemDisplayInfoMaterialRes WHERE ItemDisplayInfoID = %1").arg(m_displayId));
  if (texinfos.valid && !texinfos.empty())
  {
    QString classFilter = QString("ComponentTextureFileData.ClassID = %1").arg(CLASS_ANY);
    if (m_charModel && m_charModel->cd.isDemonHunter())
      classFilter = QString("(ComponentTextureFileData.ClassID = %1 OR ComponentTextureFileData.ClassID = %2)").arg(CLASS_DEMONHUNTER).arg(CLASS_ANY);
    
    if (queryItemInfo(QString("SELECT TextureID FROM ItemDisplayInfoMaterialRes "
                              "LEFT JOIN TextureFileData ON TextureFileDataID = TextureFileData.ID "
                              "INNER JOIN ComponentTextureFileData ON ComponentTextureFileData.ID = TextureFileData.TextureID "
                              "WHERE (ComponentTextureFileData.GenderIndex = %1 OR ComponentTextureFileData.GenderIndex = %2) "
                              "AND ItemDisplayInfoID = %3 AND %4 "
                              "ORDER BY ComponentTextureFileData.GenderIndex, ComponentTextureFileData.ClassID DESC")
                              .arg(GENDER_ANY).arg(charInfos.sexID).arg(m_displayId).arg(classFilter),
                      iteminfos))
    {
      for (uint i = 0; i < iteminfos.values.size(); i++)
      {
        GameFile * tex = GAMEDIRECTORY.getFile(iteminfos.values[i][0].toInt());
        if (tex)
        {
          CharRegions texRegion = getRegionForTexture(tex);
          // Only add one texture per region (first one in sort order):
          if (m_itemTextures.count(texRegion) < 1)
          {
            TEXTUREMANAGER.add(tex);
            m_itemTextures[texRegion] = tex;
          }
        }
      }
    }
  }

  switch (m_slot)
  {
    case CS_HEAD:
    {
      // attachments
      updateItemModel(ATT_HELMET, model[0], texture[0]);

      // geosets
      // Head: {geosetGroup[0] = 2700**, geosetGroup[1] = 2101 }
      m_itemGeosets[CG_GEOSET2700] = 1 + geosetGroup[0];
      m_itemGeosets[CG_GEOSET2100] = 1 + geosetGroup[1];
      
      // 'collections' models:
      if (model[1] != 0)
      {
        mergeModel(CS_HEAD, model[1], texture[1]);
        m_mergedModel->setGeosetGroupDisplay(CG_GEOSET2700, 1 + attachmentGeosetGroup[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_GEOSET2100, 1 + attachmentGeosetGroup[1]);
      }
      
      break;
    }
    case CS_SHOULDER:
    {
      // geosets
      // Shoulder: {geosetGroup[0] = 2601}
      m_itemGeosets[CG_GEOSET2600] = 1 + geosetGroup[0];

      // find position index value from ComponentModelFileData table
      QString query = QString("SELECT ID, PositionIndex FROM ComponentModelFileData "
                              "WHERE ID IN (%1,%2)").arg(model[0]).arg(model[1]);
      sqlResult result = GAMEDATABASE.sqlQuery(query);
      
      int leftIndex = 0;
      int rightIndex = 1;
      if (result.valid && result.values.size() > 0)
      {
        int modelid = result.values[0][0].toInt();
        int position = result.values[0][1].toInt();
        
        if (modelid == model[0])
        {
          if (position == 0)
          {
            leftIndex = 0;
            rightIndex = 1;
          }
          else
          {
            leftIndex = 1;
            rightIndex = 0;
          }
        }
        else
        {
          if (position == 0)
          {
            leftIndex = 1;
            rightIndex = 0;
          }
          else
          {
            leftIndex = 0;
            rightIndex = 1;
          }
        }
      }
      else
      {
        LOG_ERROR << "Impossible to query information for item" << name() << "(id " << m_id << "- display id" << m_displayId << ") - SQL ERROR";
        LOG_ERROR << query;
      }

      LOG_INFO << "leftIndex" << leftIndex << "rightIndex" << rightIndex;

      // left shoulder
      updateItemModel(ATT_LEFT_SHOULDER, model[leftIndex], texture[leftIndex]);

      // right shoulder
      updateItemModel(ATT_RIGHT_SHOULDER, model[rightIndex], texture[rightIndex]);
      
      break;
    }
    case CS_BOOTS:
    {
      // geosets
      // Boots: {geosetGroup[0] = 501, geosetGroup[1] = 2000*}
      m_itemGeosets[CG_BOOTS] = 1 + geosetGroup[0];
      // geoset group 20 (CG_HDFEET) is handled a bit differently, according to wowdev.wiki:
      if (geosetGroup[1] == 0)
        m_itemGeosets[CG_HDFEET] = 2;
      else if (geosetGroup[1] > 0)
        m_itemGeosets[CG_HDFEET] = geosetGroup[1];
      // else ? should we do anything if geosetGroup[1] < 0?

      // 'collections' models:
      if (model[0] != 0)
      {
        mergeModel(CS_BOOTS, model[0], texture[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_BOOTS, 1 + attachmentGeosetGroup[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_HDFEET, 1 + attachmentGeosetGroup[1]);
      }

      break;
    }
    case CS_BELT:
    {
 
      // geosets
      // Waist: {geosetGroup[0] = 1801}
      m_itemGeosets[CG_BELT] = 1 + geosetGroup[0];

      // buckle model
      updateItemModel(ATT_BELT_BUCKLE, model[0], texture[0]);
      
      // 'collections' models:
      if (model[1] != 0)
      {
        mergeModel(CS_BELT, model[1], texture[1]);
        m_mergedModel->setGeosetGroupDisplay(CG_BELT, 1 + attachmentGeosetGroup[0]);
      }
     
      break;
    }
    case CS_PANTS:
    {
      // geosets
      // Pants: {geosetGroup[0] = 1101, geosetGroup[1] = 901, geosetGroup[2] = 1301}
      m_itemGeosets[CG_PANTS2] = 1 + geosetGroup[0];
      m_itemGeosets[CG_KNEEPADS] = 1 + geosetGroup[1];
      m_itemGeosets[CG_TROUSERS] = 1 + geosetGroup[2];
      
      // 'collections' models:
      if (model[0] != 0)
      {
        mergeModel(CS_PANTS, model[0], texture[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_PANTS2, 1 + attachmentGeosetGroup[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_KNEEPADS, 1 + attachmentGeosetGroup[1]);
        m_mergedModel->setGeosetGroupDisplay(CG_TROUSERS, 1 + attachmentGeosetGroup[2]);
      }

      break;
    }
    case CS_SHIRT:
    case CS_CHEST:
    {
      // geosets
      // Chest: {geosetGroup[0] = 801, geosetGroup[1] = 1001, geosetGroup[2] = 1301, geosetGroup[3] = 2201, geosetGroup[4] = 2801}
      m_itemGeosets[CG_WRISTBANDS] = 1 + geosetGroup[0];
      m_itemGeosets[CG_PANTS] = 1 + geosetGroup[1];
      m_itemGeosets[CG_TROUSERS] = 1 + geosetGroup[2];
      m_itemGeosets[CG_GEOSET2200] = 1 + geosetGroup[3];
      m_itemGeosets[CG_GEOSET2800] = 1 + geosetGroup[4];
      
      // 'collections' models:
      if (model[0] != 0)
      {
        mergeModel(CS_CHEST, model[0], texture[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_WRISTBANDS, 1 + attachmentGeosetGroup[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_PANTS, 1 + attachmentGeosetGroup[1]);
        m_mergedModel->setGeosetGroupDisplay(CG_TROUSERS, 1 + attachmentGeosetGroup[2]);
        m_mergedModel->setGeosetGroupDisplay(CG_GEOSET2200, 1 + attachmentGeosetGroup[3]);
        m_mergedModel->setGeosetGroupDisplay(CG_GEOSET2800, 1 + attachmentGeosetGroup[4]);
      }
      
      break;
    }
    case CS_BRACERS:
    {
      // nothing specific for bracers
      break;
    }
    case CS_GLOVES:
    {
      // geosets 
      // Gloves: {geosetGroup[0] = 401, geosetGroup[1] = 2301}
      m_itemGeosets[CG_GLOVES] = 1 + geosetGroup[0];
      m_itemGeosets[CG_HANDS] = 1 + geosetGroup[1];
      
      // 'collections' models:
      if (model[0] != 0)
      {
        mergeModel(CS_GLOVES, model[0], texture[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_GLOVES, 1 + attachmentGeosetGroup[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_HANDS, 1 + attachmentGeosetGroup[1]);
      }
    
      break;
    }
    case CS_HAND_RIGHT:
    case CS_HAND_LEFT:
    {
      updateItemModel(((m_slot == CS_HAND_RIGHT) ? ATT_RIGHT_PALM : ATT_LEFT_PALM), model[0], texture[0]);
      break;
    }
    case CS_CAPE:
    {
      GameFile * tex = GAMEDIRECTORY.getFile(texture[0]);
      if (tex)
      {
        TEXTUREMANAGER.add(tex);
        m_itemTextures[getRegionForTexture(tex)] = tex;
      }
      
      // geosets
      // Cape: {geosetGroup[0] = 1501}
      m_itemGeosets[CG_CAPE] = 1 + geosetGroup[0];
      
      // 'collections' models:
      if (model[0] != 0)
      {
        mergeModel(CS_CAPE, model[0], texture[0]);
        m_mergedModel->setGeosetGroupDisplay(CG_CAPE, 1 + attachmentGeosetGroup[0]);
      }
      
      break;
    }
    case CS_TABARD:
    {
      if (isCustomizableTabard())
      {
        m_charModel->td.showCustom = true;
        m_itemGeosets[CG_TABARD] = 2;

        GameFile * texture = m_charModel->td.GetBackgroundTex(CR_TORSO_UPPER);
        if (texture)
        {
          TEXTUREMANAGER.add(texture);
          m_itemTextures[CR_TABARD_1] = texture;
        }

        texture = m_charModel->td.GetBackgroundTex(CR_TORSO_LOWER);
        if (texture)
        {
          TEXTUREMANAGER.add(texture);
          m_itemTextures[CR_TABARD_2] = texture;
        }

        texture = m_charModel->td.GetIconTex(CR_TORSO_UPPER);
        if (texture)
        {
          TEXTUREMANAGER.add(texture);
          m_itemTextures[CR_TABARD_3] = texture;
        }

        texture = m_charModel->td.GetIconTex(CR_TORSO_LOWER);
        if (texture)
        {
          TEXTUREMANAGER.add(texture);
          m_itemTextures[CR_TABARD_4] = texture;
        }

        texture = m_charModel->td.GetBorderTex(CR_TORSO_UPPER);
        if (texture)
        {
          TEXTUREMANAGER.add(texture);
          m_itemTextures[CR_TABARD_5] = texture;
        }

        texture = m_charModel->td.GetBorderTex(CR_TORSO_LOWER);
        if (texture)
        {
          TEXTUREMANAGER.add(texture);
          m_itemTextures[CR_TABARD_6] = texture;
        }
      }
      else
      {
        m_charModel->td.showCustom = false;

        // geosets
        // Tabard: {geosetGroup[0] = 1201}
        m_itemGeosets[CG_TABARD] = 1 + geosetGroup[0];
      }

      break;
    }
    case CS_QUIVER:
      break;
    default:
      break;
  }
}

void WoWItem::refresh()
{
  if (m_id == 0) // no item equipped, give up
    return;

  // merge model if any
  if (m_mergedModel != 0)
    m_charModel->mergeModel(m_mergedModel, -1);

  // update geoset values
  for (auto it : m_itemGeosets)
  {
    if ((m_slot != CS_BOOTS) && // treat boots geoset in a special case - cf CS_BOOTS
        (m_slot != CS_PANTS)) // treat trousers geoset in a special case - cf CS_PANTS
    {
      m_charModel->cd.geosets[it.first] = it.second;
      /*
      if (m_mergedModel != 0)
        m_mergedModel->setGeosetGroupDisplay(it.first, 1);
      */
    }
  }

  // attach items if any
  if (m_charModel->attachment)
  {
    if ((m_slot != CS_HAND_RIGHT) && // treat right hand attachment in a special case - cf CS_HAND_RIGHT
        (m_slot != CS_HAND_LEFT))    // treat left hand attachment in a special case - cf CS_HAND_LEFT
    {
      m_charModel->attachment->delSlot(m_slot);
      for (auto it : m_itemModels)
        m_charModel->attachment->addChild(it.second, it.first, m_slot);
    }
  }

  // add textures if any
  if ((m_slot != CS_BOOTS) &&  // treat boots texturing in a special case - cf CS_BOOTS
      (m_slot != CS_GLOVES) && // treat gloves texturing in a special case - cf CS_GLOVES 
      (m_slot != CS_TABARD) && // treat tabard texturing in a special case - cf CS_TABARD 
      (m_slot != CS_CAPE))     // treat cape texturing in a special case - cf CS_CAPE 
  {
    for (auto it : m_itemTextures)
      m_charModel->tex.addLayer(it.second, it.first, SLOT_LAYERS[m_slot]);
  }
  

  switch (m_slot)
  {
    case CS_HEAD:
    {
      // nothing specific for head items
      break;
    }
    case CS_SHOULDER:
    {
      // nothing specific for shoulder items
      break;
    }
    case CS_HAND_RIGHT:
    {
      if (m_charModel->attachment)
      {
        m_charModel->attachment->delSlot(CS_HAND_RIGHT);

        std::map<POSITION_SLOTS, WoWModel *>::iterator it = m_itemModels.find(ATT_RIGHT_PALM);
        if (it != m_itemModels.end())
        {
          int attachement = ATT_RIGHT_PALM;
          const ItemRecord &item = items.getById(m_id);
          if (m_charModel->bSheathe &&  item.sheath != SHEATHETYPE_NONE)
          {
            // make the weapon cross
            if (item.sheath == ATT_LEFT_BACK_SHEATH)
              attachement = ATT_RIGHT_BACK_SHEATH;
            if (item.sheath == ATT_LEFT_BACK)
              attachement = ATT_RIGHT_BACK;
            if (item.sheath == ATT_LEFT_HIP_SHEATH)
              attachement = ATT_RIGHT_HIP_SHEATH;
          }

          if (m_charModel->bSheathe)
            m_charModel->charModelDetails.closeRHand = false;
          else
            m_charModel->charModelDetails.closeRHand = true;

          m_charModel->attachment->addChild(it->second, attachement, m_slot);
        }
      }
      break;
    }
    case CS_HAND_LEFT:
    {
      if (m_charModel->attachment)
      {
        m_charModel->attachment->delSlot(CS_HAND_LEFT);

        std::map<POSITION_SLOTS, WoWModel *>::iterator it = m_itemModels.find(ATT_LEFT_PALM);
        if (it != m_itemModels.end())
        {
          const ItemRecord &item = items.getById(m_id);
          int attachement = ATT_LEFT_PALM;

          if (item.type == IT_SHIELD)
            attachement = ATT_LEFT_WRIST;

          if (m_charModel->bSheathe &&  item.sheath != SHEATHETYPE_NONE)
            attachement = (POSITION_SLOTS)item.sheath;

          if (m_charModel->bSheathe || item.type == IT_SHIELD)
            m_charModel->charModelDetails.closeLHand = false;
          else
            m_charModel->charModelDetails.closeLHand = true;

          Vec3D rot(0.0f, 0.0f, 0.0f);
          Vec3D pos(0.0f, 0.0f, 0.0f);

          // if (display_flags & 0x100) then item should be mirrored when in left hand:
          bool mirror = (display_flags & 0x100);
          m_charModel->attachment->addChild(it->second, attachement, m_slot, 1.0f, rot, pos, mirror);
        }
      }
      break;
    }
    case CS_BELT:
    {
      // nothing specific for belt items
      break;
    }
    case CS_BOOTS:
    {
      for (auto it : m_itemGeosets)
      {
        if (it.first != CG_BOOTS)
        {
          m_charModel->cd.geosets[it.first] = it.second;
          /*
          if (m_mergedModel != 0)
            m_mergedModel->setGeosetGroupDisplay(it.first, 1);
          */
        }
      }

      auto geoIt = m_itemGeosets.find(CG_BOOTS);

      if (geoIt != m_itemGeosets.end())
      {
        // don't render boots behind robe
        WoWItem * chestItem = m_charModel->getItem(CS_CHEST);
        if (chestItem->m_type != IT_ROBE) // maybe not handle when geoIt->second = 5 ?
        {
          m_charModel->cd.geosets[CG_BOOTS] = geoIt->second;
          /*
          if (m_mergedModel != 0)
            m_mergedModel->setGeosetGroupDisplay(CG_BOOTS, 1);
          */
        }
      }

      std::map<CharRegions, GameFile *>::iterator texIt = m_itemTextures.find(CR_LEG_LOWER);
      if (texIt != m_itemTextures.end())
        m_charModel->tex.addLayer(texIt->second, CR_LEG_LOWER, SLOT_LAYERS[m_slot]);

      if (!m_charModel->cd.showFeet)
      {
        texIt = m_itemTextures.find(CR_FOOT);
        if (texIt != m_itemTextures.end())
          m_charModel->tex.addLayer(texIt->second, CR_FOOT, SLOT_LAYERS[m_slot]);
      }
      break;
    }
    case CS_PANTS:
    {
      for (auto it : m_itemGeosets)
      {
        if (it.first != CG_TROUSERS)
        {
          m_charModel->cd.geosets[it.first] = it.second;
          /*
          if (m_mergedModel != 0)
            m_mergedModel->setGeosetGroupDisplay(it.first, 1);
          */
        }
      }
      
      std::map<CharGeosets, int>::iterator geoIt = m_itemGeosets.find(CG_TROUSERS);

      if (geoIt != m_itemGeosets.end())
      {
        // apply trousers geosets only if character is not already wearing a robe
        const ItemRecord &item = items.getById(m_charModel->getItem(CS_CHEST)->id());

        if (item.type != IT_ROBE)
        {
          m_charModel->cd.geosets[CG_TROUSERS] = geoIt->second;
          /*
          if (m_mergedModel)
            m_mergedModel->setGeosetGroupDisplay(CG_TROUSERS, 1);
          */
        }
      }

      break;
    }
    case CS_SHIRT:
    case CS_CHEST:
    {
      // nothing specific for shirt & chest items
      break;
    }
    case CS_BRACERS:
    {
      // nothing specific for bracers items
      break;
    }
    case CS_GLOVES:
    {
      std::map<CharRegions, GameFile *>::iterator texIt = m_itemTextures.find(CR_ARM_LOWER);

      int layer = SLOT_LAYERS[m_slot];

      // if we are wearing a robe, render gloves first in texture compositing
      // only if GeoSetGroup1 is 0 (from item displayInfo db) which corresponds to stored geoset equals to 1
      WoWItem * chestItem = m_charModel->getItem(CS_CHEST);
      if ((chestItem->m_type == IT_ROBE) && (m_charModel->cd.geosets[CG_GLOVES] == 1))
        layer = SLOT_LAYERS[CS_CHEST] - 1;

      if (texIt != m_itemTextures.end())
        m_charModel->tex.addLayer(texIt->second, CR_ARM_LOWER, layer);

      texIt = m_itemTextures.find(CR_HAND);
      if (texIt != m_itemTextures.end())
        m_charModel->tex.addLayer(texIt->second, CR_HAND, layer);
      break;
    }
    case CS_CAPE:
    {
      std::map<CharRegions, GameFile *>::iterator it = m_itemTextures.find(CR_CAPE);
      if (it != m_itemTextures.end())
        m_charModel->updateTextureList(it->second, TEXTURE_OBJECT_SKIN);
      break;
    }
    case CS_TABARD:
    {
      if (isCustomizableTabard())
      {
        std::map<CharRegions, GameFile *>::iterator it = m_itemTextures.find(CR_TABARD_1);
        if (it != m_itemTextures.end())
          m_charModel->tex.addLayer(it->second, CR_TORSO_UPPER, SLOT_LAYERS[m_slot]);

        it = m_itemTextures.find(CR_TABARD_2);
        if (it != m_itemTextures.end())
          m_charModel->tex.addLayer(it->second, CR_TORSO_LOWER, SLOT_LAYERS[m_slot]);

        it = m_itemTextures.find(CR_TABARD_3);
        if (it != m_itemTextures.end())
          m_charModel->tex.addLayer(it->second, CR_TORSO_UPPER, SLOT_LAYERS[m_slot]);

        it = m_itemTextures.find(CR_TABARD_4);
        if (it != m_itemTextures.end())
          m_charModel->tex.addLayer(it->second, CR_TORSO_LOWER, SLOT_LAYERS[m_slot]);

        it = m_itemTextures.find(CR_TABARD_5);
        if (it != m_itemTextures.end())
          m_charModel->tex.addLayer(it->second, CR_TORSO_UPPER, SLOT_LAYERS[m_slot]);

        it = m_itemTextures.find(CR_TABARD_6);
        if (it != m_itemTextures.end())
          m_charModel->tex.addLayer(it->second, CR_TORSO_LOWER, SLOT_LAYERS[m_slot]);

      }
      else
      {
        std::map<CharRegions, GameFile *>::iterator it = m_itemTextures.find(CR_TORSO_UPPER);
        if (it != m_itemTextures.end())
          m_charModel->tex.addLayer(it->second, CR_TORSO_UPPER, SLOT_LAYERS[m_slot]);

        it = m_itemTextures.find(CR_TORSO_LOWER);
        if (it != m_itemTextures.end())
          m_charModel->tex.addLayer(it->second, CR_TORSO_LOWER, SLOT_LAYERS[m_slot]);
      }
      break;
    }
    default:
      break;
  }
}

bool WoWItem::isCustomizableTabard() const
{
  return (m_id == 5976 || // Guild Tabard
          m_id == 69209 || // Illustrious Guild Tabard
          m_id == 69210);  // Renowned Guild Tabard
}

void WoWItem::save(QXmlStreamWriter & stream)
{
  stream.writeStartElement("item");

  stream.writeStartElement("slot");
  stream.writeAttribute("value", QString::number(m_slot));
  stream.writeEndElement();

  stream.writeStartElement("id");
  stream.writeAttribute("value", QString::number(m_id));
  stream.writeEndElement();

  stream.writeStartElement("displayId");
  stream.writeAttribute("value", QString::number(m_displayId));
  stream.writeEndElement();

  stream.writeStartElement("level");
  stream.writeAttribute("value", QString::number(m_level));
  stream.writeEndElement();

  if (isCustomizableTabard())
    m_charModel->td.save(stream);

  stream.writeEndElement(); // item
}

void WoWItem::load(QString & f)
{
  QFile file(f);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    LOG_ERROR << "Fail to open" << f;
    return;
  }

  QXmlStreamReader reader;
  reader.setDevice(&file);

  int nbValuesRead = 0;
  while (!reader.atEnd() && nbValuesRead != 3)
  {
    if (reader.isStartElement())
    {
      if (reader.name() == "slot")
      {
        unsigned int slot = reader.attributes().value("value").toString().toUInt();

        if (slot == m_slot)
        {
          while (!reader.atEnd() && nbValuesRead != 3)
          {
            if (reader.isStartElement())
            {
              if (reader.name() == "id")
              {
                int id = reader.attributes().value("value").toString().toInt();
                nbValuesRead++;
                if (id != -1)
                  setId(id);
              }

              if (reader.name() == "displayId")
              {
                int id = reader.attributes().value("value").toString().toInt();
                nbValuesRead++;
                if (m_id == -1)
                  setDisplayId(id);
              }

              if (reader.name() == "level")
              {
                int level = reader.attributes().value("value").toString().toInt();
                nbValuesRead++;
                setLevel(level);
              }
            }
            reader.readNext();
          }
        }
      }
    }
    reader.readNext();
  }

  if (isCustomizableTabard()) // look for extra tabard details
  {
    reader.readNext();
    while (reader.isStartElement() == false)
      reader.readNext();

    if (reader.name() == "TabardDetails")
    {
      m_charModel->td.load(reader);
      load(); // refresh tabard textures
    }
  }
}

void WoWItem::updateItemModel(POSITION_SLOTS pos, int modelId, int textureId)
{
  LOG_INFO << __FUNCTION__ << pos << modelId << textureId;

  if (modelId == 0)
    return;

  WoWModel *m = new WoWModel(GAMEDIRECTORY.getFile(modelId), true);

  if (m->ok)
  {
    for (uint i = 0; i < m->geosets.size(); i++)
      m->showGeoset(i, true);

    m_itemModels[pos] = m;
    GameFile * texture = GAMEDIRECTORY.getFile(textureId);
    if (texture)
      m->updateTextureList(texture, TEXTURE_OBJECT_SKIN);
    else
      LOG_ERROR << "Error during item update" << m_id << "(display id" << m_displayId << "). Texture" << textureId << "can't be loaded";
  }
  else
  {
    LOG_ERROR << "Error during item update" << m_id << "(display id" << m_displayId << "). Model" << modelId << "can't be loaded";
  }
}

void WoWItem::mergeModel(CharSlots slot, int modelId, int textureId)
{
  if (modelId == 0)
    return;

  m_mergedModel = new WoWModel(GAMEDIRECTORY.getFile(modelId), true);

  if (m_mergedModel->ok)
  {
    GameFile * texture = GAMEDIRECTORY.getFile(textureId);
    if (texture)
    {
      m_mergedModel->updateTextureList(texture, TEXTURE_OBJECT_SKIN);
      m_charModel->updateTextureList(texture, TEXTURE_OBJECT_SKIN);
    }
    else
      LOG_ERROR << "Error during item update" << m_id << "(display id" << m_displayId << "). Texture" << textureId << "can't be loaded";

    for (uint i = 0; i < m_mergedModel->geosets.size(); i++)
      m_mergedModel->hideAllGeosets();
  }
  else
  {
    LOG_ERROR << "Error during item update" << m_id << "(display id" << m_displayId << "). Model" << modelId << "can't be loaded";
  }
}

CharRegions WoWItem::getRegionForTexture(GameFile * file) const
{
  CharRegions result = CR_UNK8;

  if (file)
  {
    QString fullname = file->fullname();

    if (fullname.contains("armlowertexture", Qt::CaseInsensitive))
    {
      result = CR_ARM_LOWER;
    }
    else if (fullname.contains("armuppertexture", Qt::CaseInsensitive))
    {
      result = CR_ARM_UPPER;
    }
    else if (fullname.contains("foottexture", Qt::CaseInsensitive))
    {
      result = CR_FOOT;
    }
    else if (fullname.contains("handtexture", Qt::CaseInsensitive))
    {
      result = CR_HAND;
    }
    else if (fullname.contains("leglowertexture", Qt::CaseInsensitive))
    {
      result = CR_LEG_LOWER;
    }
    else if (fullname.contains("leguppertexture", Qt::CaseInsensitive))
    {
      result = CR_LEG_UPPER;
    }
    else if (fullname.contains("torsolowertexture", Qt::CaseInsensitive))
    {
      result = CR_TORSO_LOWER;
    }
    else if (fullname.contains("torsouppertexture", Qt::CaseInsensitive))
    {
      result = CR_TORSO_UPPER;
    }
    else if (fullname.contains("cape", Qt::CaseInsensitive))
    {
      result = CR_CAPE;
    }
    else
    {
      LOG_ERROR << "Unable to determine region for texture" << fullname << " - item" << m_id << "displayid" << m_displayId;
    }
  }

  return result;
}

bool WoWItem::queryItemInfo(QString & query, sqlResult & result) const
{
  result = GAMEDATABASE.sqlQuery(query);
 
  if (!result.valid || result.values.empty())
  {
    LOG_ERROR << "Impossible to query information for item" << name() << "(id " << m_id << "- display id" << m_displayId << ") - SQL ERROR";
    LOG_ERROR << query;
    return false;
  }

  return true;
}

int WoWItem::getCustomModelId(size_t index)
{
  sqlResult infos;
  if (!queryItemInfo(QString("SELECT ModelID FROM ItemDisplayInfo "
                             "LEFT JOIN ModelFileData ON %1 = ModelFileData.ID "
                             "WHERE ItemDisplayInfo.ID = %2").arg((index == 0)?"Model1":"Model2").arg(m_displayId),
                     infos))
    return 0;

  // if there is only one result, return model id:
  if (infos.values.size() == 1)
    return infos.values[0][0].toInt();

  // if there are multiple values, check by race and sex:
  QStringList idList;
  for (auto it : infos.values)
    idList << it[0];
  QString idListStr = idList.join(", ");
  idListStr = "(" + idListStr + ")";
  
  RaceInfos charInfos;
  RaceInfos::getCurrent(m_charModel, charInfos);

  QString classFilter = QString("ClassID = %1").arg(CLASS_ANY);
  if (m_charModel && m_charModel->cd.isDemonHunter())
    classFilter = QString("(ClassID = %1 OR ClassID = %2)").arg(CLASS_DEMONHUNTER).arg(CLASS_ANY);
  
  // It looks like shoulders are always in pairs, with PositionIndex values 0 and 1.
  // Depending on index (model 1 or 2) we sort the PositionIndex differently so one will
  // return left and one right shoulder. Noting this in case in the future it turns out
  // this assumption isn't always right - Wain
  QString positionSort = ((index == 0) ? "" : "DESC");
  
  // Order all queries by GenderIndex to ensure definite genders have priority over generic ones,
  // and ClassID descending to ensure Demon Hunter textures have priority over regular ones, for DHs only:
  sqlResult iteminfos;
  QString query = QString("SELECT ID, PositionIndex FROM ComponentModelFileData "
                          "WHERE RaceID = %1 AND (GenderIndex = %2 OR GenderIndex = %3) "
                          "AND ID IN %4 AND %5 "
                          "ORDER BY GenderIndex, ClassID DESC, PositionIndex %6")
                          .arg(charInfos.raceID).arg(charInfos.sexID).arg(GENDER_ANY).arg(idListStr).arg(classFilter).arg(positionSort);
  if (queryItemInfo(query, iteminfos))
    return iteminfos.values[0][0].toInt();

  // Failed to find model for that specific race and sex, so check fallback race:
  int fallbackRaceID = 0;
  int fallbackSex = -1;
  if (charInfos.sexID == GENDER_MALE)
  {
    fallbackRaceID = charInfos.MaleModelFallbackRaceID;
    fallbackSex = charInfos.MaleModelFallbackSex;
  }
  else if (charInfos.sexID == GENDER_FEMALE)
  {
    fallbackRaceID = charInfos.FemaleModelFallbackRaceID;
    fallbackSex = charInfos.FemaleModelFallbackSex;
  }
  if (fallbackRaceID > 0)
  {
    query = QString("SELECT ID, PositionIndex FROM ComponentModelFileData "
                    "WHERE RaceID = %1 AND (GenderIndex = %2 OR GenderIndex = %3) "
                    "AND ID IN %4 AND %5 "
                    "ORDER BY GenderIndex, ClassID DESC, PositionIndex %6")
                    .arg(fallbackRaceID).arg(fallbackSex).arg(GENDER_NONE).arg(idListStr).arg(classFilter).arg(positionSort);
    if (queryItemInfo(query, iteminfos))
      return iteminfos.values[0][0].toInt();
  }
  
  // We still didn't find the model, so check for RACE_ANY (race = 0) items:
  // Note: currently all race = 0 entries are also gender = 2, but we probably
  // shouldn't assume it will stay that way, so check for both gender values:
  query = QString("SELECT ID, PositionIndex FROM ComponentModelFileData "
                  "WHERE RaceID = %1 AND (GenderIndex = %2 OR GenderIndex = %3) "
                  "AND ID IN %4 AND %5 "
                  "ORDER BY GenderIndex, ClassID DESC, PositionIndex %6")
                  .arg(RACE_ANY).arg(fallbackSex).arg(GENDER_NONE).arg(idListStr).arg(classFilter).arg(positionSort);
  if (queryItemInfo(query, iteminfos))
    return iteminfos.values[0][0].toInt();

  return 0;
}

int WoWItem::getCustomTextureId(size_t index)
{
  sqlResult infos;
  if (!queryItemInfo(QString("SELECT TextureID FROM ItemDisplayInfo "
                             "LEFT JOIN TextureFileData ON %1 = TextureFileData.ID "
                             "WHERE ItemDisplayInfo.ID = %2").arg((index == 0)?"TextureItemID1":"TextureItemID2").arg(m_displayId),
                     infos))
    return 0;

  // if there is only one result, return texture id:
  if (infos.values.size() == 1)
    return infos.values[0][0].toInt();

  // if there are multiple values, check by race and sex:
  QStringList idList;
  for (auto it : infos.values)
    idList << it[0];
  QString idListStr = idList.join(", ");
  idListStr = "(" + idListStr + ")";
  
  RaceInfos charInfos;
  RaceInfos::getCurrent(m_charModel, charInfos);
  
  QString classFilter = QString("ClassID = %1").arg(CLASS_ANY);
  if (m_charModel && m_charModel->cd.isDemonHunter())
    classFilter = QString("(ClassID = %1 OR ClassID = %2)").arg(CLASS_DEMONHUNTER).arg(CLASS_ANY);
  
  // Order all queries by GenderIndex to ensure definite genders have priority over generic ones,
  // and ClassID descending to ensure Demon Hunter textures have priority over regular ones, for DHs only:
  sqlResult iteminfos; 
  QString query = QString("SELECT ID FROM ComponentTextureFileData "
                          "WHERE RaceID = %1 AND (GenderIndex = %2 OR GenderIndex = %3) "
                          "AND ID IN %4 AND %5 "
                          "ORDER BY GenderIndex, ClassID DESC")
                          .arg(charInfos.raceID).arg(charInfos.sexID).arg(GENDER_ANY).arg(idListStr).arg(classFilter);
  if (queryItemInfo(query, iteminfos))
    return iteminfos.values[0][0].toInt();

  // Failed to find model for that specific race and sex, so check fallback race:
  int fallbackRaceID = 0;
  int fallbackSex = -1;
  if (charInfos.sexID == GENDER_MALE)
  {
    fallbackRaceID = charInfos.MaleTextureFallbackRaceID;
    fallbackSex = charInfos.MaleTextureFallbackSex;
  }
  else if (charInfos.sexID == GENDER_FEMALE)
  {
    fallbackRaceID = charInfos.FemaleTextureFallbackRaceID;
    fallbackSex = charInfos.FemaleTextureFallbackSex;
  }
  if (fallbackRaceID > 0)
  {
    query = QString("SELECT ID FROM ComponentTextureFileData "
                    "WHERE RaceID = %1 AND (GenderIndex = %2 OR GenderIndex = %3) "
                    "AND ID IN %4 AND %5 "
                    "ORDER BY GenderIndex, ClassID DESC")
                    .arg(fallbackRaceID).arg(fallbackSex).arg(GENDER_ANY).arg(idListStr).arg(classFilter);
    if (queryItemInfo(query, iteminfos))
      return iteminfos.values[0][0].toInt();
  }
  
  // We still didn't find the model, so check for RACE_ANY (race = 0) items: 
  query = QString("SELECT ID FROM ComponentTextureFileData "
                  "WHERE RaceID = %1 AND (GenderIndex = %2 OR GenderIndex = %3) "
                  "AND ID IN %4 AND %5 "
                  "ORDER BY GenderIndex, ClassID DESC")
                  .arg(RACE_ANY).arg(charInfos.sexID).arg(GENDER_ANY).arg(idListStr).arg(classFilter);
  if (queryItemInfo(query, iteminfos))
    return iteminfos.values[0][0].toInt();

  return 0;
}
