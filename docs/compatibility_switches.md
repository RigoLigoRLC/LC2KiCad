# Parser Arguments
## Syntax
Parser arguments comes in the following syntax:

`-a option[=value][,option[=value][,option[=value][...]]]`

`-a option[:value][,option[:value][,option[:value][...]]]`

You can use a mixture of colons and equal signs. In the internals of the program every colon is literally replaced with equal signs.

We call each `option=value` pair a **parser argument** here. A parser argument contains the option and the value you've given. The `option` is not case sensitive, as they would be converted to uppercase literally internally. The `value` would be evaluated as a 32-bit signed integer.

All occurrences of `-a PARSER_ARGS` argument would be used. The ones comes later will replace the ones already evaluated.

Notice that the `=value` is actually optional. Some options don't require you passing a value to it. So you can pads no value at all. However, for those options that require a value being passed to it, passing no value would cause the program use a default value 0 for this specific option.

## Switches

### This part is being rewritten.
