# Compatibility Switches
## Syntax
Compatibility switches comes in the following syntax:

`-Cname[=value][,name[=value][,name[=value][...]]]`

`-Cname[:value][,name[:value][,name[:value][...]]]`

You can use a mixture of colons and equal signs. In the internals of the program every colon is literally replaced with equal signs.

We call each `name=value` pair a **compatibility switch** here. A compatibility switch contains the name of the actual switch and the value you've given. The `name` is not case sensitive, as they would be converted to uppercase literally internally. The `value` would be evaluated as a double-precision floating-point number.

Only the first occurrence of `-Ccompatibility_switches` argument would be used. If the parser see additional ones, they would be ignored and a warning message should be displayed.

Notice that the `=value` is actually optional. Some switches don't require you passing a value to it, and passing a value to such switches has no point, and it wouldn't be interpreted; so you can choose passing no value at all. However, for those switches that require a value being passed to it, passing no value would have no effect; when the program interprets this switch, it would use the default value for this specific switch.

## Switches

### NativeCopperArc (NCA)

Use the arc copper provided by KiCad natively. **If there is native copper arc and circle in the stable version, the default choice would be changed to 3.**

*This feature is only available in currently development version of KiCad, so it is normally not enabled. Before the finish of development of KiCad native copper arc support, this function won't take any effect and won't be implemented.*

| Value           | Behavior                                                     |
| --------------- | ------------------------------------------------------------ |
| 0               | Don't use native arc copper support.                         |
| **1 (Default)** | Use native arc copper support. This will suppress **SCA** and **SCC**. |

### SegCopperArc (SCA) & SegCopperCircle (SCC)

When exporting for lower version of KiCad which doesn't natively support arcs or circles on copper layer, segments mimicking arcs and circles would be used for copper arcs and circles.

| Value              | Behavior                                                     |
| ------------------ | :----------------------------------------------------------- |
| 0                  | Disabled, and use graphical arcs and circles on copper layer for the task. |
| 0<value<3          | Value specifies the maximum length of each segment.          |
| value>=3           | Value specifies the number of segments in an arc or a circle. |
| **0.15 (Default)** | ---                                                          |

### UseSvgFont (USF)

Create the text outline with the SVG data stored in the PCB and footprint document.

Notice: if no valid SVG data was found in the file, the program would use KiCad native stroke font text, whose size could be wrong.

| Value           | Behavior               |
| --------------- | ---------------------- |
| 0               | Disable this function. |
| **1 (Default)** | Enable this function.  |

### PcbBoundingOperations (PBO) & SchematicsBoundingOperations (SBO)

Choose if the program determines the bounding box of the PCB or schematics, and choose the action it does.

Program will convert the value to an integer and read its lowest four bits.

Structure looks like this: `XXXXABCD`

Bits A and B are responsible for schematics, and bits C and D are responsible for PCBs. When A and B are both 0, program won't find the bounding box of the schematics; and when they're both 1, program will take the higher bit as the final decision. This is same for C and D.

| Value                     | Behavior                                                     |
| ------------------------- | ------------------------------------------------------------ |
| **Bit "A" = 1 (Default)** | Find the bounding box and put everything at the top left corner of the page. |
| Bit "B" = 1               | Find the bounding box and put everything at the center of the page. |
| **Bit "C" = 1 (Default)** | Find the bounding box and put everything at the top left corner of the page. |
| Bit "D" = 1               | Find the bounding box and put everything at the center of the page. |

### SolidFillConversion (SFC)

KiCad don't have a solid filled region element with net like EasyEDA. This switch specifies how the program process the actual solid region.

A value was reserved for KiCad native solid filled region in case we get access to it in the future. **If there is a solid region in the stable version, the default choice would be changed to 3.**

| Value                  | Behavior                                                     |
| ---------------------- | ------------------------------------------------------------ |
| 0                      | Left the solid region out unconverted.                       |
| **1 (Default)**        | Convert the solid region into a flood fill region.           |
| 2                      | Convert the solid region into a graphical region without nets on copper layer. |
| ~~***3 (Reserved)***~~ | ~~Convert the solid region into the KiCad native solid filled region.~~ |

### UseInterpreterArcs (UIA) *(No value required)*

Normally the program uses its built-in Simple LC SVG Path Sentimentalizer program for the arcs. This could cause performance issues since this is not a proper or standard implementation of SVG path. With this switch, when the input file do have the corresponding type of arc data (only SVG path and no other information), the program will interpret the data in a hard-coded way.

**This switch may fail to work with newer files, and the support of this switch could be removed at any moment.**