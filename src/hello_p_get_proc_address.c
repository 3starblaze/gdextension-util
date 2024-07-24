#include "../godot-headers/gdextension_interface.h"
#include <stdio.h>
#include <stdbool.h>

/* NOTE: We don't need to wait for initialization but we need to provide a function either way. */
void godot_noop(void *_userdata, GDExtensionInitializationLevel _p_level) {
  return;
}

GDExtensionBool
godot_entry(
  GDExtensionInterfaceGetProcAddress p_get_proc_address,
  const GDExtensionClassLibraryPtr _p_library,
  GDExtensionInitialization *r_initialization
) {
  r_initialization->minimum_initialization_level = GDEXTENSION_INITIALIZATION_SCENE;
  r_initialization->userdata = NULL;
  r_initialization->initialize = godot_noop;
  r_initialization->deinitialize = godot_noop;

  GDExtensionInterfaceGetGodotVersion get_godot_version = p_get_proc_address("get_godot_version");
  GDExtensionGodotVersion godot_version;

  get_godot_version(&godot_version);

  printf("GDExtensionGodotVersion godot_version\n");
  printf(".major: %d\n", godot_version.major);
  printf(".minor: %d\n", godot_version.minor);
  printf(".patch: %d\n", godot_version.patch);
  printf(".string: %s\n", godot_version.string);

  return true;
}
