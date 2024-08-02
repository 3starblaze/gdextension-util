#include "../godot-headers/gdextension_interface.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define STORE_GD_EXTENSION(str_name) gd_extension.str_name = (void *)p_get_proc_address(#str_name);
#define IS_GODOT_64_BIT (true)
#define MY_CUSTOM_CLASS_NAME ("MyCustomNode")
#define MY_CUSTOM_CLASS_PARENT ("Node")


struct {
  GDExtensionInterfaceClassdbConstructObject classdb_construct_object;
  GDExtensionInterfaceClassdbRegisterExtensionClass2 classdb_register_extension_class2;
  GDExtensionInterfaceStringNameNewWithUtf8Chars string_name_new_with_utf8_chars;
  GDExtensionInterfaceObjectSetInstance object_set_instance;
  GDExtensionInterfaceVariantGetPtrDestructor variant_get_ptr_destructor;
} gd_extension;

struct {
  struct {
    GDExtensionPtrDestructor string_name;
  } destructor;
  struct {
    GDExtensionClassLibraryPtr p_library;
  } misc;
} gd_extension_helper;

GDExtensionStringNamePtr construct_string_name(const char *c_string) {
  void *res = malloc(IS_GODOT_64_BIT ? 8 : 4);
  gd_extension.string_name_new_with_utf8_chars(res, c_string);
  return res;
}

void destruct_string_name(GDExtensionStringNamePtr p) {
  gd_extension_helper.destructor.string_name(p);
}

typedef struct {
  GDExtensionObjectPtr godot_object;
} my_custom_class_t;

GDExtensionObjectPtr my_custom_class_init(void *userdata) {
  my_custom_class_t *my_instance = malloc(sizeof(my_custom_class_t));

  void *my_class_string_name = construct_string_name(MY_CUSTOM_CLASS_NAME);
  void *parent_class_string_name = construct_string_name(MY_CUSTOM_CLASS_PARENT);

  my_instance->godot_object = gd_extension.classdb_construct_object(parent_class_string_name);
  gd_extension.object_set_instance(my_instance->godot_object, my_class_string_name, my_instance);

  destruct_string_name(my_class_string_name);
  destruct_string_name(parent_class_string_name);

  printf("Hey, instancing is done!\n");

  return my_instance->godot_object;
}

void my_custom_class_deinit(void *userdata, GDExtensionClassInstancePtr p_instance) {
  if (p_instance == NULL) return;

  my_custom_class_t *my_instance = p_instance;
  free(my_instance);

  printf("my_custom_class is going down, goodbye world!\n");
}

// NOTE: We can only call this when Node has been loaded in ClassDB (during
// GDEXTENSION_INITIALIZATION_SCENE)
void register_my_custom_class() {
  GDExtensionClassCreationInfo2 class_info = {
    .is_virtual = false,
    .is_abstract = false,
    .is_exposed = true,
    .set_func = NULL,
    .get_func = NULL,
    .get_property_list_func = NULL,
    .free_property_list_func = NULL,
    .property_can_revert_func = NULL,
    .property_get_revert_func = NULL,
    .validate_property_func = NULL,
    .notification_func = NULL,
    .to_string_func = NULL,
    .reference_func = NULL,
    .unreference_func = NULL,
    .create_instance_func = my_custom_class_init,
    .free_instance_func = my_custom_class_deinit,
    .recreate_instance_func = NULL,
    .get_virtual_func = NULL,
    .get_virtual_call_data_func = NULL,
    .call_virtual_with_data_func = NULL,
    .get_rid_func = NULL,
    .class_userdata = NULL,
  };

  void *my_class_string_name = construct_string_name(MY_CUSTOM_CLASS_NAME);
  void *parent_class_string_name = construct_string_name(MY_CUSTOM_CLASS_PARENT);

  gd_extension.classdb_register_extension_class2(gd_extension_helper.misc.p_library,
                                                 my_class_string_name,
                                                 parent_class_string_name,
                                                 &class_info);

  destruct_string_name(my_class_string_name);
  destruct_string_name(parent_class_string_name);
}

void godot_initialize(void *userdata, GDExtensionInitializationLevel p_level) {
  if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
    register_my_custom_class();
    return;
  }
}

void godot_deinitialize(void *userdata, GDExtensionInitializationLevel p_level) {
  return;
}

GDExtensionBool
godot_entry(
  GDExtensionInterfaceGetProcAddress p_get_proc_address,
  const GDExtensionClassLibraryPtr p_library,
  GDExtensionInitialization *r_initialization
) {
  r_initialization->minimum_initialization_level = GDEXTENSION_INITIALIZATION_SCENE;
  r_initialization->userdata = NULL;
  r_initialization->initialize = godot_initialize;
  r_initialization->deinitialize = godot_deinitialize;

  STORE_GD_EXTENSION(classdb_construct_object);
  STORE_GD_EXTENSION(classdb_register_extension_class2);
  STORE_GD_EXTENSION(string_name_new_with_utf8_chars);
  STORE_GD_EXTENSION(object_set_instance);
  STORE_GD_EXTENSION(variant_get_ptr_destructor);

  gd_extension_helper.misc.p_library = p_library;

  gd_extension_helper.destructor.string_name
    = gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING_NAME);

  return true;
}
