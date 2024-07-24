#include "../godot-headers/gdextension_interface.h"
#include <stdio.h>
#include <stdbool.h>

void godot_initialize(void *userdata, GDExtensionInitializationLevel p_level) {
  if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
    printf("hello from scene!\n");
    return;
  } else if (p_level == GDEXTENSION_INITIALIZATION_EDITOR) {
    printf("hello from editor!\n");
    return;
  }
}

void godot_deinitialize(void *userdata, GDExtensionInitializationLevel p_level) {
  if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
    printf("goodbye from scene!\n");
    return;
  } else if (p_level == GDEXTENSION_INITIALIZATION_EDITOR) {
    printf("goodbye from editor!\n");
    return;
  }
}

GDExtensionBool
godot_entry(
  GDExtensionInterfaceGetProcAddress _p_get_proc_address,
  const GDExtensionClassLibraryPtr _p_library,
  GDExtensionInitialization *r_initialization
) {
  r_initialization->minimum_initialization_level = GDEXTENSION_INITIALIZATION_SCENE;
  r_initialization->userdata = NULL;
  r_initialization->initialize = godot_initialize;
  r_initialization->deinitialize = godot_deinitialize;

  return true;
}
