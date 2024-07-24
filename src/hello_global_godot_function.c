#include "../godot-headers/gdextension_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define IS_GODOT_64_BIT (true)
#define IS_GODOT_USING_LARGE_WORLD_COORDINATES (false)

struct {
  GDExtensionInterfaceVariantGetPtrUtilityFunction variant_get_ptr_utility_function;
  GDExtensionInterfaceStringNameNewWithUtf8Chars string_name_new_with_utf8_chars;
  GDExtensionInterfaceVariantGetPtrDestructor variant_get_ptr_destructor;
} gd_extension;

struct {
  struct {
    GDExtensionPtrDestructor string_name;
  } destructor;
} gd_extension_helper;

GDExtensionStringNamePtr construct_string_name(const char *c_string) {
  void *res = malloc(IS_GODOT_64_BIT ? 8 : 4);
  gd_extension.string_name_new_with_utf8_chars(res, c_string);
  return res;
}

void destruct_string_name(GDExtensionStringNamePtr p) {
  gd_extension_helper.destructor.string_name(p);
}

GDExtensionUninitializedVariantPtr alloc_variant() {
  return malloc(IS_GODOT_USING_LARGE_WORLD_COORDINATES ? 40 : 24);
}

void print_rad_to_deg_result() {
  GDExtensionStringNamePtr rad_to_deg_string_name = construct_string_name("rad_to_deg");
  /*
    From `godot-headers/extension_api.json`:

    {
      "name": "rad_to_deg",
      "return_type": "float",
      "category": "math",
      "is_vararg": false,
      "hash": 2140049587,
      "arguments": [
        {
          "name": "rad",
          "type": "float"
        }
      ]
    }
  */
  GDExtensionPtrUtilityFunction rad_to_deg
    = gd_extension.variant_get_ptr_utility_function(rad_to_deg_string_name, 2140049587);

  double raw_rad = 3.14;
  const GDExtensionConstTypePtr args[] = { &raw_rad };
  double result;
  rad_to_deg(&result, args, 1);

  printf("3.14rad is equal to %f deg\n", result);
}

void godot_initialize(void *userdata, GDExtensionInitializationLevel p_level) {
  if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
    print_rad_to_deg_result();
  }
}

void godot_deinitialize(void *userdata, GDExtensionInitializationLevel p_level) {
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
  r_initialization->initialize = godot_initialize;
  r_initialization->deinitialize = godot_deinitialize;

  gd_extension.variant_get_ptr_utility_function
    = (void *)p_get_proc_address("variant_get_ptr_utility_function");
  gd_extension.string_name_new_with_utf8_chars
    = (void *)p_get_proc_address("string_name_new_with_utf8_chars");
  gd_extension.variant_get_ptr_destructor
    = (void *)p_get_proc_address("variant_get_ptr_destructor");

  gd_extension_helper.destructor.string_name
    = gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING_NAME);

  return true;
}
