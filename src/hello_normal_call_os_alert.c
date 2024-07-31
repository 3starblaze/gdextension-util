#include "../godot-headers/gdextension_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define STORE_GD_EXTENSION(str_name) gd_extension.str_name = (void *)p_get_proc_address(#str_name);
#define IS_GODOT_64_BIT (true)
#define IS_GODOT_USING_LARGE_WORLD_COORDINATES (false)

struct {
  GDExtensionInterfaceGlobalGetSingleton global_get_singleton;
  GDExtensionInterfaceStringNameNewWithUtf8Chars string_name_new_with_utf8_chars;
  GDExtensionInterfaceVariantGetPtrDestructor variant_get_ptr_destructor;
  GDExtensionInterfaceClassdbGetMethodBind classdb_get_method_bind;
  GDExtensionInterfaceStringNewWithUtf8Chars string_new_with_utf8_chars;
  GDExtensionInterfaceObjectMethodBindCall object_method_bind_call;
  GDExtensionInterfaceGetVariantFromTypeConstructor get_variant_from_type_constructor;
  GDExtensionInterfaceVariantGetType variant_get_type;
  GDExtensionInterfaceVariantDestroy variant_destroy;
} gd_extension;

struct {
  struct {
    GDExtensionVariantFromTypeConstructorFunc string;
  } to_variant;
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

  GDExtensionMethodBindPtr alert_method_bind
    = gd_extension.classdb_get_method_bind(os_string_name, alert_string_name, 1783970740);

  GDExtensionStringPtr body_string = construct_string("Hello normal OS.alert call!");
  // NOTE: I am making a byte array because I don't want to malloc memory that I need to free
  unsigned char body_string_variant[IS_GODOT_USING_LARGE_WORLD_COORDINATES ? 40 : 24];
  gd_extension_helper.to_variant.string(&body_string_variant, body_string);

  const GDExtensionConstVariantPtr args[] = { &body_string_variant };

  GDExtensionCallError call_error;
  unsigned char return_variant[IS_GODOT_USING_LARGE_WORLD_COORDINATES ? 40 : 24];

  gd_extension.object_method_bind_call(alert_method_bind,
                                       os_object,
                                       args,
                                       1,
                                       &return_variant,
                                       &call_error);


  if (call_error.error != GDEXTENSION_CALL_OK) {
    fprintf(stderr, "you messed up in calling OS.alert!\n");
    fprintf(stderr, "code: %d\n", call_error.error);
    fprintf(stderr, "argument: %d\n", call_error.argument);
    fprintf(stderr, "expected: %d\n", call_error.expected);
    exit(1);
  }

  printf("Response variant type is %d\n", gd_extension.variant_get_type(&return_variant));

  destruct_string_name(os_string_name);
  destruct_string_name(alert_string_name);
  destruct_string(body_string);
  gd_extension.variant_destroy(&body_string_variant);
  gd_extension.variant_destroy(&return_variant);
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
  STORE_GD_EXTENSION(string_new_with_utf8_chars);
  STORE_GD_EXTENSION(string_new_with_utf8_chars);
  STORE_GD_EXTENSION(object_method_bind_call);
  STORE_GD_EXTENSION(get_variant_from_type_constructor);
  STORE_GD_EXTENSION(variant_get_type);
  STORE_GD_EXTENSION(variant_destroy);


  gd_extension_helper.to_variant.string
    = gd_extension.get_variant_from_type_constructor(GDEXTENSION_VARIANT_TYPE_STRING);
  gd_extension_helper.destructor.string_name
    = gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING_NAME);
  gd_extension_helper.destructor.string
    = gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING);

  return true;
}
