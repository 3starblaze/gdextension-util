# GDExtension Util


## What is this?

A reference repository that documents Godot's GDExtension API and provides some C boilerplate.

## Documentation

### Introduction

In order to make the integration, you should familiarize yourself with two files `gde-header` (`godot-headers/gdextension_interface.h`) and `gde-api` (`godot-headers/extension_api.json`). `gde-header` lists all sorts of types that are mostly unique to Godot + C interop. `gde-api` mostly describes the regular Godot API that is used for game development. 

Everything begins when Godot calls a function that matches the `GDExtensionInitializationFunction` type. `gde-header` offers some explanation about this function. In order to begin writing the C code, we will import the header and define the entry function as `godot_entry`.

```c
#include "../godot-headers/gdextension_interface.h"

GDExtensionBool
godot_entry(
  GDExtensionInterfaceGetProcAddress p_get_proc_address,
  const GDExtensionClassLibraryPtr p_library,
  GDExtensionInitialization *r_initialization
) {

}
```

First we need to understand all three parameters, in order to understand how we can tell Godot what needs to be done. 

The first parameter `p_get_proc_address` is a function that takes a `const char *p_function_name` and returns a function. This function is the only way to interact with Godot because `gde-header` doesn't list any functions that can be called, it's just typedefs. If you search for `@name` in `gde-header`, you will see several *special* function typedefs that have docblocks. Those are public functions you can call but first you have to retrieve them via `p_get_proc_address`.

The second parameter `p_library` is mostly used for defining custom classes that will appear in Godot once your C library is loaded.

The third parameter, `r_initialization` is used to configure initialization. Let's look inside `gde-header`.
```c
// ...
typedef enum {
	GDEXTENSION_INITIALIZATION_CORE,
	GDEXTENSION_INITIALIZATION_SERVERS,
	GDEXTENSION_INITIALIZATION_SCENE,
	GDEXTENSION_INITIALIZATION_EDITOR,
	GDEXTENSION_MAX_INITIALIZATION_LEVEL,
} GDExtensionInitializationLevel;

typedef struct {
	/* Minimum initialization level required.
	 * If Core or Servers, the extension needs editor or game restart to take effect */
	GDExtensionInitializationLevel minimum_initialization_level;
	/* Up to the user to supply when initializing */
	void *userdata;
	/* This function will be called multiple times for each initialization level. */
	void (*initialize)(void *userdata, GDExtensionInitializationLevel p_level);
	void (*deinitialize)(void *userdata, GDExtensionInitializationLevel p_level);
// ...
```

Once Godot starts, it initializes in layers, first core, second servers, third scene and finally editor (if the editor is loaded). These initialization layers roughly correspond to those in [Godot's architecture diagram](https://docs.godotengine.org/en/stable/contributing/development/core_and_modules/godot_architecture_diagram.html) page. 

![Godot architecture diagram](https://docs.godotengine.org/en/stable/_images/architecture_diagram.jpg)

If you don't know what to choose, you only need to care about *scene* (which is always loaded) or *editor* (which is only loaded when the editor is loaded). Set `r_initialization`'s `initialize` and `deinitialize`, compare the `p_level` to `GDEXTENSION_INITIALIZATION_SCENE` or `GDEXTENSION_INITIALIZATION_SCENE` and do the work. If you don't wait for initialization (for example, you want to do your work in the entry function) then Godot classes cannot be used and you will encounter `ERROR: Cannot get class 'Node'.` or similar.

With this knowledge you can make a simple "Hello world" program that will be called when a Godot project is loaded. Once you compile `hello_gdextension.c` and run the minimal project, you can see print statements "hello from scene!" and "hello from editor!" which means that everything works as expected! If you close Godot normally (and don't terminate with `Ctrl-C`), you will see "goodbye from editor!" and "goodbye from scene!". 

```bash
./build.py src/hello_gdextension.c # Build the first example
godot mvp-godot-project/project.godot # Open the project
```

### Hello `p_get_proc_address`

`src/hello_p_get_proc_address.c` contains a simple usage of `p_get_process_address` in order to retrieve the current version of Godot.

```bash
./build.py src/hello_p_get_proc_address.c
godot mvp-godot-project/project.godot
```

### Hello global Godot function

`src/hello_global_godot_function.c` shows how you can call a Godot function that is defined in the global scope. You can find documentation in [@GlobalScope](https://docs.godotengine.org/en/stable/classes/class_%40globalscope.html). In our program we have chosen to call `rad_to_deg`. While it's not worth the effort to call this particular function because the formula is as simple as `rad * 180 / PI`, it's perfect for demonstration purposes because we can quickly tell if the function output is correct.

This example introduces several new concepts and thus is noticeably larger.

First of all, to keep our sanity, we have made a `gd_extension` structure that holds the global API functions. Keeping functions in a struct lets our function calls look like they belong in a namespace, e.g. `gd_extension.string_name_new_with_utf8_chars(res, c_string)`, which makes code more readable. 

Secondly, @GlobalScope functions are called **utility functions** in GDExtension and the function signatures are found in `gde-api`'s `utility_functions` field. An important concept to keep in mind is the notion of function/method signature **hash** which uniquely identifies a signature. It doesn't seem to be an important concept in global functions but there are several classes in Godot with methods that can have optional arguments or several signatures. In practical terms, if we want to call a function/method, we need to specify both *name* and *hash* which you can see in `gd_extension.variant_get_ptr_utility_function(rad_to_deg_string_name, 2140049587);`.

Thirdly, you are greeted with `StringName` which appears rather often in GDExtension. You can read [StringName documentation](https://docs.godotengine.org/en/stable/classes/class_stringname.html) in the official docs but the main idea is that StringName comparisons are really fast and the rule of thumb is that if you want to specify a class, property or anything else that needs to be found via string, it's going to be to a StringName. 

GDExtension provides a convenience function `string_name_with_utf8_chars` that can turn a regular C string into `StringName`. If we take a look at our convenience function `construct_string_name`, we see that the size depends on whether the Godot is built for 64bit (which it is). In order to figure out, how many bytes are needed, we need to look inside `gde-api`'s `builtin_class_sizes`. There are 4 build configurations: `float_32`, `float_64`, `double_32`, `double_64` which respectively correspond to regular 32-bit, regular 64-bit, large world coordinate 32-bit, large world coordinate 64bit. If we inspect the size of `StringName`, we get 4, 8, 4, 8 which means that the StringName size only depends on bit width. By default Godot is in `float_64` configuration and you would need to build other configurations separately. You may want to read [Large world coordinates](https://docs.godot.community/tutorials/physics/large_world_coordinates.html) documentation.

Since `StringName` is a Godot Variant, we need to destruct it like a Variant by first getting `variant_get_ptr_destructor` and then obtaining the destructor with `gd_extension.variant_get_ptr_destructor(GDEXTENSION_VARIANT_TYPE_STRING_NAME)`. 

Finally we can talk about the task we want to achieve -- use `rad_to_deg` to convert 3.14 radians to degrees. We first fetch the utility function via name + hash that we found in `gde-api` then we prepare arguments and destination and finally we call the function and print the results.

A potentially confusing part are the types -- `GDExtensionTypePtr`, `GDExtensionConstTypePtr`, `GDExtensionUninitializedTypePtr`. They are just regular C types that you can access directly and the types of these variables depend on the function we are using. Since `rad_to_deg` takes a float and returns a float then we have to use a C double. I know this because the size of Godot's `float` is always 8 (as per `gde-api`) and the C type that fits the bill is `double`.

Now that out of the way, we can compile and run the example and get `3.14rad is equal to 179.908748 deg` (the degrees may vary a bit) in the terminal. We have successfully called a global function. 
