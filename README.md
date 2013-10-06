Geany Code Format Plugin
========================

The Code Format plugin allows simple and precise formatting of
C, C++ and Objective-C source code using the excellent utility
`clang-format` provided as part of the `ClangTools`.

Features
--------

* Accurate lexical code formatting using Clang frontend.
* Runs `clang-format` as an external process so you can swap out and
upgrade versions at will without touching the plugin.
* Auto-formatting; never worry about formatting again, let the plugin
do all the work!
* Can format current line, current selection, current document, or
auto-format the entire document based on specified trigger characters.

Usage
-----

### UI Functionality

Activate the plugin using Geany's Plugin Manager dialog. Once activated
it will place a new `Code Format` item in the `Tools` menu inside the
main menu. You can access basic functionality from here. The items and
submenu's will automatically become enabled or disabled as certain
features cannot be used (ex. trying to format with no open documents or
documents with unsupported filetypes).

### Keybindings

There are two keybindings available, one to format the current
selection (or current line if there is no selection), the other to
format the entire document. You can set the keybindings through Geany's
main Preferences dialog in the Keybindings tab.

### Preferences

The preferences are broken into two parts. The first is a regular
`.conf` file containing your settings for the Code Format plugin
itself, like where to find the `clang-format` utility, or whether
auto-formatting is enabled. The second type of preferences are those
sepcified in `YAML` files and are read by the `clang-format` utility
to control how the code is formatted. This section discusses the former.

The user preferences are stored in a file named `code-format.conf`
found in under the Geany configuration directory inside the `plugins`
directory. The project-specific settings are stored inside the
actual Geany project file (usally ends with `.geany`).

If a project is open, the project-specific preferences take effect and
can be changed using the Project Preferences dialog accessible through
Geany's `Project` menu. If no project is opened, then the user
preferences are used and can be changed using the Plugin Preferences
dialog accessible through Geany's `Edit` menu. If you change the
user preferences while a project is open, the changes made won't take
effect until there is no project open.

In the configuration files, Code Format settings are stored in the
`[code-format]` group.

#### ClangFormat Path

This setting specifies the path to the `clang-format` utility that
is part of ClangTools. If the executable is found in the `PATH`
environment variable, you can just put `clang-format` otherwise,
you can choose the full path to the binary. When a valid executable
file is found, an "OK" icon will appear in the text box, otherwise
an "error" icon will appear. It only means that the file is found
and is executable, not that it's actually `clang-format`.

In the configuration file, this setting is known as `clang-format-path`.

#### Style

This setting controls whether to use one of the preset code formatting
styles available from `clang-format` or whether to use a Custom
`.clang-format` file to control the code formatting style. If you
select one of the presets, you can click on the `Create` button to
open a new YAML document in Geany containing the code formatting
settings based on the selected item that you can tailor to suit
your specific code formatting style.

In the configuration file, this setting is known as `style` and can
be (at present) one of `llvm`, `google`, `chromium`, `mozilla` or
`custom`.

#### Format on Save

This setting controls whether the active document will be formatted just
before Geany saves it. This is especially useful if you don't like to
enable the auto-formatting option but still want mostly-automatic
formatting of the code.

In the configuration file, this setting is known as `format-on-save`.

#### Auto-Format

This setting controls whether the current document is formatted
automatically when one of a specific set of trigger characters are
typed. Even though it is not on by default, it is recommended that
you use this feature, and only turn if off if you find it annoying
or it gets too slow on large documents.

In the configuration file, this setting is known as `auto-format`.

#### Trigger Characters

When `auto-format` is enabled, this setting controls the characters
which when typed will cause the document to be re-formatted. The
default set "`)}];`" seems to work well but you might like to
customize these a bit. You probably don't want to use "`\n`" (newline)
or similar.

In the configuration file, this setting is known as
`auto-format-trigger-chars`.

ClangFormat Information
-----------------------

[ClangFormat](http://clang.llvm.org/docs/ClangFormat.html) is part
of the [ClangTools](http://clang.llvm.org/docs/ClangTools.html) set
of utilities. You must install the `clang-format` utility in order
to use the Code Format plugin for Geany. You can configure which
`clang-format` executable gets used in the Preferences (see above).
It is **HIGHLY** recommended that you read the documentation for
`clang-format` (at the above link) before using the Code Format
plugin, in order to have any clue how it works.

It's important to note that various features documented or available
through the user interface may not actually be supported by your
version of `clang-format`. It is recommended to always use the
latest version although it seems to handle unknown options and
such gracefully.

### Configuration

If you're using one of the preset code formatting styles (see
Preferences), you basically don't have to configure anything for
`clang-format`. The presets are equivalent to running:

    $ clang-format -style=<NAME_OF_STYLE> ...

Where `<NAME_OF_STYLE>` is not equal to `file`.

If you're using a custom `.clang-format` file, you should configure it
as per ClangFormat's documentation. It's a YAML file with readable
names and you should be able to figure it out while referencing the
ClangFormat documentation (at the above link). The custom configuration
is equivalent to running:

    $ clang-format -style=file ...

The special name `file` tells `clang-format` to read `.clang-format`
files instead of using a preset.

To get started, you can base your custom configuration on one of the
presets by using the "Create" button in the preferences dialog with
the desired preset selected in the list (see Preferences). The "Create"
button is equivalent to running:

    $ clang-format -style=<SELECTED_PRESET> -dump-config

Where `<SELECTED_PRESET>` is the preset chosen in the Style list when
the button is pressed. The output of the command is placed into a
new unsaved document named `.clang-format`. You should then save it in
a directory (see below) and customize it according to your needs.

The `.clang-format` file should be saved at or above the document(s)
you want formatting to work for. For example you can put it straight
into the source directory or you can put it at the root of your
project directory and it will affect all subdirectories (unless a
certain subdirectory has a `.clang-format` file as well). Because
of this functionality, it is required that a document has been saved
on disk before so that the plugin can determine in which directory
to start looking for `.clang-format` files. If you try and use the
plugin on a new, never before saved document, the plugin simply
won't function and it will print some message to Geany's standard
output.

Author and Contact
------------------

The Code Format plugin was written by and is maintained by
Matthew Brush <matt(at)geany(dot)org>. You can report any issues
on the Github Issues page (TODO link) or provide any improvement
by making a Pull Request on Github.
