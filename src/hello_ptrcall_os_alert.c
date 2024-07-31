#include "../godot-headers/gdextension_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define STORE_GD_EXTENSION(str_name) gd_extension.str_name = (void *)p_get_proc_address(#str_name);
#define IS_GODOT_64_BIT (true)

struct {
  GDExtensionInterfaceGlobalGetSingleton global_get_singleton;
  GDExtensionInterfaceStringNameNewWithUtf8Chars string_name_new_with_utf8_chars;
  GDExtensionInterfaceVariantGetPtrDestructor variant_get_ptr_destructor;
  GDExtensionInterfaceClassdbGetMethodBind classdb_get_method_bind;
  GDExtensionInterfaceObjectMethodBindPtrcall object_method_bind_ptrcall;
  GDExtensionInterfaceStringNewWithUtf8Chars string_new_with_utf8_chars;
} gd_extension;

struct {
  struct {
    GDExtensionPtrDestructor string_name;
    GDExtensionPtrDestructor string;
  } destructor;
} gd_extension_helper;

GDExtensionStringNamePtr construct_string_name(const char *c_string) {
  void *res = malloc(IS_GODOT_64_BIT ? 8 : 4);
  gd_extension.string_name_new_with_utf8_chars(res, c_string);
  return res;
}

GDExtensionStringPtr construct_string(const char *c_string) {
  void *res = malloc(IS_GODOT_64_BIT ? 8 : 4);
  gd_extension.string_new_with_utf8_chars(res, c_string);
  return res;
}

void destruct_string_name(GDExtensionStringNamePtr p) {
  gd_extension_helper.destructor.string_name(p);
}

void destruct_string(GDExtensionStringPtr p) {
  gd_extension_helper.destructor.string(p);
}

void do_work() {
  GDExtensionStringNamePtr os_string_name = construct_string_name("OS");
  GDExtensionObjectPtr os_object = gd_extension.global_get_singleton(os_string_name);
  GDExtensionStringNamePtr alert_string_name = construct_string_name("alert");
  /*
        {
					"name": "alert",
					"is_const": false,
					"is_vararg": false,
					"is_static": false,
					"is_virtual": false,
					"hash": 1783970740,
					"hash_compatibility": [
						233059325
					],
					"arguments": [
						{
							"name": "text",
							"type": "String"
						},
						{
							"name": "title",
							"type": "String",
							"default_value": "\"Alert!\""
						}
					]
				}
  */
  GDExtensionMethodBindPtr alert_method_bind
    = gd_extension.classdb_get_method_bind(os_string_name, alert_string_name, 1783970740);

  GDExtensionStringPtr title_string = construct_string("Hello OS ptrcall!");
  GDExtensionStringPtr body_string = construct_string("The example was successful.");
  // ptr call style
  const GDExtensionConstTypePtr args[] = { body_string, title_string };
  gd_extension.object_method_bind_ptrcall(alert_method_bind, os_object, args, NULL);

  destruct_string_name(os_string_name);
  destruct_string_name(alert_string_name);
  destruct_string(title_string);
  destruct_string(body_string);
}

void godot_initialize(void *userdata, GDExtensionInitializationLevel p_level) {
  if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
    do_work();
    return;
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

  STORE_GD_EXTENSION(global_get_singleton);
  STORE_GD_EXTENSION(string_name_new_with_utf8_chars);
  STORE_GD_EXTENSION(variant_get_ptr_destructor);
  STORE_GD_EXTENSION(classdb_get_method_bind);
  STORE_GD_EXTENSION(object_method_bind_ptrcall);
  STORE_GD_EXTENSION(string_new_with_utf8_chars);

  gd_extension_helper.destructor.string_name
    = gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING_NAME);
  gd_extension_helper.destructor.string
    = gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING);

  return true;
}
