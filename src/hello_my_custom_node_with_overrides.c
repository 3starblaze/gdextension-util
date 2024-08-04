#include "../godot-headers/gdextension_interface.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define STORE_GD_EXTENSION(str_name) gd_extension.str_name = (void *)p_get_proc_address(#str_name);
#define IS_GODOT_64_BIT (true)
#define IS_GODOT_USING_LARGE_WORLD_COORDINATES (false)
#define VARIANT_SIZE (IS_GODOT_USING_LARGE_WORLD_COORDINATES ? 40 : 24)
#define MY_CUSTOM_CLASS_NAME ("MyCustomNode")
#define MY_CUSTOM_CLASS_PARENT ("Sprite2D")


struct {
  GDExtensionInterfaceClassdbConstructObject classdb_construct_object;
  GDExtensionInterfaceClassdbRegisterExtensionClass2 classdb_register_extension_class2;
  GDExtensionInterfaceClassdbGetMethodBind classdb_get_method_bind;
  GDExtensionInterfaceStringNameNewWithUtf8Chars string_name_new_with_utf8_chars;
  GDExtensionInterfaceStringNewWithUtf8Chars string_new_with_utf8_chars;
  GDExtensionInterfaceObjectSetInstance object_set_instance;
  GDExtensionInterfaceVariantGetPtrDestructor variant_get_ptr_destructor;
  GDExtensionInterfaceVariantEvaluate variant_evaluate;
  GDExtensionInterfaceGetVariantFromTypeConstructor get_variant_from_type_constructor;
  GDExtensionInterfaceGetVariantToTypeConstructor get_variant_to_type_constructor;
  GDExtensionInterfaceVariantGetPtrOperatorEvaluator variant_get_ptr_operator_evaluator;
  GDExtensionInterfaceVariantGetType variant_get_type;
  GDExtensionInterfaceObjectMethodBindPtrcall object_method_bind_ptrcall;
} gd_extension;

struct {
  struct {
    GDExtensionPtrDestructor string_name;
    GDExtensionPtrDestructor string;
  } destructor;
  struct {
    GDExtensionVariantFromTypeConstructorFunc type_double;
  } wrap;
  struct {
    GDExtensionTypeFromVariantConstructorFunc type_double;
  } unwrap;
  struct {
    GDExtensionStringNamePtr amplitude;
    GDExtensionStringNamePtr frequency;
    GDExtensionStringNamePtr _process;
    GDExtensionStringNamePtr position;
  } string_name;
  struct {
    GDExtensionClassLibraryPtr p_library;
    GDExtensionPtrOperatorEvaluator string_name_eq_op;
    GDExtensionMethodBindPtr node2d_set_position;
  } misc;
} gd_extension_helper;

#if (IS_GODOT_USING_LARGE_WORLD_COORDINATES)
typedef struct {
  double x;
  double y;
} GDVector2;
#else
typedef struct {
  float x;
  float y;
} GDVector2;
#endif

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

typedef struct {
  GDExtensionObjectPtr godot_object;
  double time_elapsed;
  struct {
    double amplitude;
    double frequency;
  } prop_state;
} my_custom_class_t;

struct {
  const char *name;
  const GDExtensionVariantType type;
} my_custom_class_props[] = {
  {
    .name = "frequency",
    .type = GDEXTENSION_VARIANT_TYPE_FLOAT,
  },
  {
    .name = "amplitude",
    .type = GDEXTENSION_VARIANT_TYPE_FLOAT,
  }
};

const GDExtensionPropertyInfo *
my_custom_class_get_property_list(
  GDExtensionClassInstancePtr p_instance,
  uint32_t *r_count
) {
  size_t n = sizeof(my_custom_class_props) / sizeof(*my_custom_class_props);
  *r_count = n;

  GDExtensionPropertyInfo *res = malloc(n * sizeof(GDExtensionPropertyInfo));

  for (size_t i = 0; i < n; i++) {
    res[i].type = my_custom_class_props[i].type;
    res[i].name = construct_string_name(my_custom_class_props[i].name);
    res[i].class_name = construct_string_name(MY_CUSTOM_CLASS_NAME);
    res[i].hint = 0; // Corresponds to no hints
    res[i].hint_string = construct_string("");
    res[i].usage = 6; // Corresponds to default usage flags
  }

  return res;
}

void
my_custom_class_free_property_list(
  GDExtensionClassInstancePtr p_instance,
  const GDExtensionPropertyInfo *p_list
) {
  size_t n = sizeof(my_custom_class_props) / sizeof(*my_custom_class_props);

  for (size_t i = 0; i < n; i++) {
    destruct_string_name((void*)p_list[i].name);
    destruct_string((void*)p_list[i].hint_string);
    destruct_string_name((void*)p_list[i].class_name);
  }

  free((void*)p_list);
}

GDExtensionObjectPtr my_custom_class_init(void *userdata) {
  my_custom_class_t *my_instance = malloc(sizeof(my_custom_class_t));

  void *my_class_string_name = construct_string_name(MY_CUSTOM_CLASS_NAME);
  void *parent_class_string_name = construct_string_name(MY_CUSTOM_CLASS_PARENT);

  my_instance->godot_object = gd_extension.classdb_construct_object(parent_class_string_name);
  my_instance->time_elapsed = 0.0;
  my_instance->prop_state.amplitude = 1.23;
  my_instance->prop_state.frequency = 2.45;
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

bool string_name_eq(const void *a, const void *b) {
  GDExtensionBool res;
  gd_extension_helper.misc.string_name_eq_op(a, b, &res);
  return res;
}

GDExtensionBool
my_custom_class_set_func(
  GDExtensionClassInstancePtr p_instance,
  GDExtensionConstStringNamePtr p_name,
  GDExtensionConstVariantPtr p_value
) {
  my_custom_class_t *my_instance = p_instance;

  if (string_name_eq(p_name, gd_extension_helper.string_name.frequency)) {
    if (gd_extension.variant_get_type(p_value) == GDEXTENSION_VARIANT_TYPE_FLOAT) {
      gd_extension_helper.unwrap.type_double(&my_instance->prop_state.frequency, (void *)p_value);
      return true;
    } else {
      return false;
    }
  }

  if (string_name_eq(p_name, gd_extension_helper.string_name.amplitude)) {
    if (gd_extension.variant_get_type(p_value) == GDEXTENSION_VARIANT_TYPE_FLOAT) {
      gd_extension_helper.unwrap.type_double(&my_instance->prop_state.amplitude, (void *)p_value);
      return true;
    } else {
      return false;
    }
  }

  return false;
}

GDExtensionBool
my_custom_class_get_func(
  GDExtensionClassInstancePtr p_instance,
  GDExtensionConstStringNamePtr p_name,
  GDExtensionVariantPtr r_ret
) {
  my_custom_class_t *my_instance = p_instance;

  if (string_name_eq(p_name, gd_extension_helper.string_name.frequency)) {
    gd_extension_helper.wrap.type_double(r_ret, &(my_instance->prop_state.frequency));
    return true;
  }

  if (string_name_eq(p_name, gd_extension_helper.string_name.amplitude)) {
    gd_extension_helper.wrap.type_double(r_ret, &(my_instance->prop_state.amplitude));
    return true;
  }

  return false;
}

void
my_custom_class__process_override(
   GDExtensionClassInstancePtr p_instance,
   const GDExtensionConstTypePtr *p_args,
   GDExtensionTypePtr r_ret
) {
  my_custom_class_t *my_instance = p_instance;
  my_instance->time_elapsed += *((double*)(p_args[0]));

  double t = my_instance->time_elapsed;
  double A = my_instance->prop_state.amplitude;
  double w = my_instance->prop_state.frequency;

  const GDVector2 new_position = {
    .x = 0,
    .y = A * sin(w * t),
  };

  GDExtensionConstTypePtr args[] = { &new_position };

  gd_extension.object_method_bind_ptrcall(gd_extension_helper.misc.node2d_set_position,
                                          my_instance->godot_object,
                                          args,
                                          NULL);

  r_ret = NULL;
}

GDExtensionClassCallVirtual
my_custom_class_get_virtual(
   void *p_class_userdata,
   GDExtensionConstStringNamePtr p_name
) {
  if (string_name_eq(p_name, gd_extension_helper.string_name._process)) {
    return my_custom_class__process_override;
  }
  return NULL;
}

// NOTE: We can only call this when Node has been loaded in ClassDB (during
// GDEXTENSION_INITIALIZATION_SCENE)
void register_my_custom_class() {
  GDExtensionClassCreationInfo2 class_info = {
    .is_virtual = false,
    .is_abstract = false,
    .is_exposed = true,
    .set_func = my_custom_class_set_func,
    .get_func = my_custom_class_get_func,
    .get_property_list_func = my_custom_class_get_property_list,
    .free_property_list_func = my_custom_class_free_property_list,
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
    .get_virtual_func = my_custom_class_get_virtual,
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
    gd_extension_helper.string_name.amplitude = construct_string_name("amplitude");
    gd_extension_helper.string_name.frequency = construct_string_name("frequency");
    gd_extension_helper.string_name._process = construct_string_name("_process");
    gd_extension_helper.string_name.position = construct_string_name("position");

    void *node2d_string_name = construct_string_name("Node2D");
    void *set_position_string_name = construct_string_name("set_position");

    gd_extension_helper.misc.node2d_set_position
      = gd_extension.classdb_get_method_bind(node2d_string_name,
                                             set_position_string_name,
                                             743155724);

    destruct_string_name(node2d_string_name);
    destruct_string_name(set_position_string_name);

    register_my_custom_class();
    return;
  }
}

void godot_deinitialize(void *userdata, GDExtensionInitializationLevel p_level) {
  if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
    destruct_string_name(gd_extension_helper.string_name.amplitude);
    destruct_string_name(gd_extension_helper.string_name.frequency);
    destruct_string_name(gd_extension_helper.string_name._process);
    destruct_string_name(gd_extension_helper.string_name.position);
  }
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
  STORE_GD_EXTENSION(classdb_get_method_bind);
  STORE_GD_EXTENSION(string_name_new_with_utf8_chars);
  STORE_GD_EXTENSION(string_new_with_utf8_chars);
  STORE_GD_EXTENSION(object_set_instance);
  STORE_GD_EXTENSION(variant_get_ptr_destructor);
  STORE_GD_EXTENSION(variant_evaluate);
  STORE_GD_EXTENSION(get_variant_from_type_constructor);
  STORE_GD_EXTENSION(get_variant_to_type_constructor);
  STORE_GD_EXTENSION(variant_get_ptr_operator_evaluator);
  STORE_GD_EXTENSION(variant_get_type);
  STORE_GD_EXTENSION(object_method_bind_ptrcall);

  gd_extension_helper.wrap.type_double
    = gd_extension.get_variant_from_type_constructor(GDEXTENSION_VARIANT_TYPE_FLOAT);

  gd_extension_helper.unwrap.type_double
    = gd_extension.get_variant_to_type_constructor(GDEXTENSION_VARIANT_TYPE_FLOAT);

  gd_extension_helper.misc.p_library = p_library;
  gd_extension_helper.misc.string_name_eq_op
    = gd_extension.variant_get_ptr_operator_evaluator(GDEXTENSION_VARIANT_OP_EQUAL,
                                                      GDEXTENSION_VARIANT_TYPE_STRING_NAME,
                                                      GDEXTENSION_VARIANT_TYPE_STRING_NAME);

  gd_extension_helper.destructor.string_name
    = gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING_NAME);
  gd_extension_helper.destructor.string
    = gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING);

  return true;
}
