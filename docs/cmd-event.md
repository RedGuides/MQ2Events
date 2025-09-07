---
tags:
  - command
---

# /event

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/event [option] [<name> <trigger text | command text>]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Adds, lists, deletes, and controls all events
<!--cmd-desc-end-->

## Options

| Option | Description |
|--------|-------------|
| `(no option)` | Displays syntax and help text |
| `settrigger <name> <trigger text>` | creates a custom trigger, will need to add a command to this &lt;name&gt; for it to work |
| `setcommand <name> <command text>` | creates a custom command, will need to add a trigger to this &lt;name&gt; for it to work |
| `load` | reloads the INI file |
| `delete <name>` | deletes the named event |
| `on|off` | turns all events on or off |
| `list` | lists all events |
