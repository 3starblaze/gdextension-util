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
