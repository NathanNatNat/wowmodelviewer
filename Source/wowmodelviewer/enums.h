#ifndef ENUMS_H
#define ENUMS_H

enum ObjectID {

  // Model Canvas
  ID_TIMER = 2000,

  // Model Viewer Frame
  ID_MODELVIEWERFRAME, // = 2000,
  ID_FILE_MODEL_INFO,
  ID_EXPORT_MODEL,
  ID_FILE_RESETLAYOUT,
  ID_FILE_EXIT,
  ID_STATUS_REFRESH_TIMER,

  // File List
  ID_FILELIST_FRAME,
  ID_FILELIST,
  ID_FILELIST_SEARCH,
  ID_FILELIST_CONTENT,
  ID_FILELIST_FILTER,
  ID_FILELIST_PLAY,
  ID_FILELIST_VIEW,
  ID_FILELIST_SAVE,

  ID_SHOW_FILE_LIST,
  ID_SHOW_ANIM,
  ID_SHOW_CHAR,
  ID_SHOW_VIEW,
  ID_SHOW_LIGHT,
  ID_SHOW_MODEL,
  ID_SHOW_MODELBANK,

  ID_SHOW_MASK,

  ID_VIEW_NPC,
  ID_VIEW_ITEM,

  ID_LOAD_WOW,
  ID_FILE_VIEWLOG,

  //ID_SHOW_BONES,
  ID_SHOW_BOUNDS,
  //ID_SHOW_PARTICLES,
  //ID_SHOW_WIREFRAME,
  ID_USE_CAMERA,

  ID_CAMERA,
  ID_CAM_FRONT,
  ID_CAM_SIDE,
  ID_CAM_BACK,
  ID_CAM_ISO,
  ID_CAM_RESET,

  ID_LT_AMBIENT,
  ID_LT_DIRECTIONAL,
  ID_LT_MODEL,
  ID_LT_COLOR,
  ID_LT_DIRECTION,
  ID_LT_TRUE,
  ID_LT_SAVE,
  ID_LT_LOAD,
  
  ID_BACKGROUND,
  ID_BG_COLOR,
  ID_SKYBOX,
  ID_SHOW_GRID,
  ID_CANVASSIZE,

  // Square Aspects
  ID_CANVASS120,
  ID_CANVASS512,
  ID_CANVASS1024,

  // Fullscreen Aspects
  ID_CANVASF480,
  ID_CANVASF600,
  ID_CANVASF768,
  ID_CANVASF864,
  ID_CANVASF1200,

  // Widescreen Aspects
  ID_CANVASW480,
  ID_CANVASW720,
  ID_CANVASW1080,

  // Misc Aspects
  ID_CANVASM768,    // 1280x768
  ID_CANVASM1200,    // 1920x1200

  ID_ZOOM_IN,
  ID_ZOOM_OUT,
  ID_OPENGL_DEBUG,

  ID_ENCHANTS,

  ID_DEFAULT_DOODADS,
  ID_USE_ANTIALIAS,
  ID_USE_ENVMAP,
  ID_USE_HWACC,
  ID_SHOW_SETTINGS,
  //ID_RESET,
  
  ID_CHECKFORUPDATE,
  ID_LANGUAGE,
  ID_HELP,
  ID_ABOUT,

  ID_MOUNT_CHARACTER,

  ID_SAVE_TEMP1,
  ID_SAVE_TEMP2,
  ID_SAVE_TEMP3,
  ID_SAVE_TEMP4,
  ID_LOAD_TEMP1,
  ID_LOAD_TEMP2,
  ID_LOAD_TEMP3,
  ID_LOAD_TEMP4,

  // -------------------------------------
  // GIF Exporter frame
  ID_GIF_FRAME, // = 3500,
  ID_GIFSTART,
  ID_GIFEXIT,
  ID_GIFFILENAME,
  ID_GIFCURFRAME,
  ID_GIFTOTALFRAME,
  ID_GIFTRANSPARENT,
  ID_GIFDIFFUSE,
  ID_GIFSHRINK,
  ID_GIFGREYSCALE,
  ID_PNGSEQ,

  // ------------------------------------
  // Anim Frame
  ID_ANIM_FRAME,// = 3000,
  ID_PAUSE,
  ID_PLAY,
  ID_STOP,
  ID_CLEARANIM,
  ID_LOADANIM,
  ID_ADDANIM,
  ID_PREVANIM,
  ID_NEXTANIM,
  ID_OLDSTYLE,
  ID_LOOPS,
  ID_ANIM,
  ID_ANIM_SECONDARY,
  ID_ANIM_SECONDARY_TEXT,
  ID_ANIM_LOCK,
  ID_ANIM_NEXT,
  ID_SKIN,
  ID_BLP_SKIN1,
  ID_BLP_SKIN2,
  ID_BLP_SKIN3,
  ID_SHOW_BLP_SKINLIST,
  ID_ITEMSET,
  ID_SPEED,
  ID_FRAME,
  ID_PAUSE_MOUTH,
  ID_ANIM_MOUTH,
  ID_SPEED_MOUTH,

  // ---------------------------------------------
  // Light Frame
  ID_LIGHT_FRAME, // = 3950,
  ID_LIGHTSEL,
  ID_LIGHTENABLED,
  ID_LIGHTRELATIVE,
  ID_LIGHTCOLOUR,
  ID_LIGHTAMBIENCE,
  ID_LIGHTDIFFUSE,
  ID_LIGHTSPECULAR,
  ID_LIGHTPOSX,
  ID_LIGHTPOSY,
  ID_LIGHTPOSZ,
  ID_LIGHTTARX,
  ID_LIGHTTARY,
  ID_LIGHTTARZ,
  ID_LIGHTCINTENSITY,
  ID_LIGHTLINTENSITY,
  ID_LIGHTQINTENSITY,
  ID_LIGHTALPHA,
  ID_LIGHTRESET,
  ID_LIGHTPOSITIONAL,
  ID_LIGHTSPOT,
  ID_LIGHTDIRECTIONAL,

  // ----------------------------------------
  // Model Control Frame
  ID_MODEL_FRAME,
  ID_MODEL_NAME,
  ID_MODEL_LOD,
  ID_MODEL_ALPHA,
  ID_MODEL_SCALE,
  ID_MODEL_BONES,
  ID_MODEL_BOUNDS,
  ID_MODEL_RENDER,
  ID_MODEL_WIREFRAME,
  ID_MODEL_PARTICLES,
  ID_MODEL_TEXTURE,
  ID_MODEL_GEOSETS,
  ID_MODEL_X,
  ID_MODEL_Y,
  ID_MODEL_Z,
  ID_MODEL_ROT_X,
  ID_MODEL_ROT_Y,
  ID_MODEL_ROT_Z,
  ID_MODEL_SIZE,
  ID_MODEL_PC_REPLACE, // use replacement colours for particles, where applicable
  ID_MODEL_PC_START_11, // For replacable particle colours, start, mid and end, IDs 11, 12 and 13
  ID_MODEL_PC_MID_11,
  ID_MODEL_PC_END_11,
  ID_MODEL_PC_START_12,
  ID_MODEL_PC_MID_12,
  ID_MODEL_PC_END_12,
  ID_MODEL_PC_START_13,
  ID_MODEL_PC_MID_13,
  ID_MODEL_PC_END_13,

  // ----------------------------------------
  // Model Bank Control Frame
  ID_MODELBANK_FRAME,
  ID_MODELBANK_NAME,
  ID_MODELBANK_ADD,
  ID_MODELBANK_REMOVE,
  ID_MODELBANK_DISPLAY,

  // -----------------------------------------
  // effects/enhants frame
  ID_ENCHANTSFRAME, // = 3600,
  ID_ENCHANTSOK,
  ID_ENCHANTSCANCEL,

  // -----------------------------------------
  // Image Control Frame
  ID_IMAGE_FRAME,
  ID_IMAGE_FILENAME,
  ID_IMAGE_CANVASWIDTH,
  ID_IMAGE_CANVASHEIGHT,
  ID_IMAGE_LOCKASPECT,
  ID_IMAGE_SAVE,
  ID_IMAGE_CANCEL,

  // Sound Control Frame
  //ID_SOUND_FRAME,
  //ID_SOUND_PLAY,
  //ID_SOUND_STOP,

  // --------------------------------------
  // Char control frame
  ID_CHAR_FRAME, // = 3400,

  ID_TABARD_ICON,
  ID_TABARD_ICONCOLOR,
  ID_TABARD_BORDER,
  ID_TABARD_BORDERCOLOR,
  ID_TABARD_BACKGROUND,

  ID_SHOW_UNDERWEAR,
  ID_SHOW_EARS,
  ID_SHOW_HAIR,
  ID_SHOW_FACIALHAIR,
  ID_SHOW_FEET,
  ID_AUTOHIDE_GEOSETS_FOR_HEAD_ITEMS,
  ID_SHEATHE,

  ID_CHAREYEGLOW,
  ID_CHAREYEGLOW_NONE,
  ID_CHAREYEGLOW_DEFAULT,
  ID_CHAREYEGLOW_DEATHKNIGHT,

  ID_SAVE_EQUIPMENT,
  ID_LOAD_EQUIPMENT,
  ID_CLEAR_EQUIPMENT,

  ID_SAVE_CHAR,
  ID_LOAD_CHAR,
  ID_IMPORT_CHAR, // From Blizzards Armory website

  ID_LOAD_SET,
  ID_LOAD_START,

  ID_MOUNT,

  ID_CHAR_RANDOMISE,

  ID_EQUIPMENT = 5000
};

enum {
  LIGHT_AMBIENT,
  LIGHT_DYNAMIC,
  LIGHT_MODEL_ONLY
};

enum {
  LIGHT_POSITIONAL,
  LIGHT_SPOT,
  LIGHT_DIRECTIONAL
};


enum {
  UPDATE_ITEM,
  UPDATE_SET,
  UPDATE_START,
  UPDATE_MOUNT,
  UPDATE_CREATURE_ITEM,
  UPDATE_NPC,
  UPDATE_SINGLE_ITEM
};

// Model Export Options
// General (Page 1)
enum {
  MEO_CHECK_PRESERVE_DIR,
  MEO_CHECK_USE_WMV_POSROT,
  MEO_CHECK_SCALE_TO_REALWORLD,

  NUM_MEO1_CHECK
};
// Lightwave (Page 2)
enum {
  MEO_CHECK_PRESERVE_LWDIR,
  MEO_CHECK_LW_ALWAYSWRITESCENEFILE,
  MEO_CHECK_LW_EXPORTLIGHTS,
  MEO_CHECK_LW_EXPORTDOODADS,
  MEO_CHECK_LW_EXPORTCAMERAS,
  MEO_CHECK_LW_EXPORTBONES,

  NUM_MEO2_CHECK
};
// X3D (Page 3)
enum {
    MEO_CHECK_EXPORT_ANIMATION,
    MEO_CHECK_CENTER_MODEL,

    NUM_MEO3_CHECK
};

// End Model Export Options

enum {
  SPIN_TABARD_ICON = 0,
  SPIN_TABARD_ICONCOLOR,
  SPIN_TABARD_BORDER,
  SPIN_TABARD_BORDERCOLOR,
  SPIN_TABARD_BACKGROUND,

  NUM_TABARD_BTNS
};

/*
enum WOW_LOCALE {
  enUS = 0,
  koKR,
  frFR,
  deDE,
  zhCN,
  zhTW,
  esES,
  esMX,
  ruRU,

  NUM_LOCALES = 16
};
*/

enum GRAPHIC_EFFECTS {
  FRAGMENT_SHADER,
  VERTEX_SHADER,
  GLSL,
  COMPRESSED_TEXTURES,
  MULTI_TEXTURES,
  DRAW_RANGE_ELEMENTS,
  POINT_SPRITES,
  //bool supportShaders;//  = false;
  ANTI_ALIAS,
  VERTEX_BUFFER,
  PIXEL_BUFFER,
  FRAME_BUFFER,
  NON_POWEROFTWO,
  OPENGL_20,
  WGLPIXELFORMAT,

  NUM_EFFECTS
};

#endif

