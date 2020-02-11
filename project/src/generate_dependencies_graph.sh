#!/bin/bash
# ============================================================================ #
# Author: Tancredi-Paul Grozav <paul@grozav.info>
# ============================================================================ #
echo "digraph G" &&
echo "{" &&
echo "  \"asm_start\" -> \"kernel\"" &&

# Look in all .c and .h source files
egrep -nir "^#include" *.{c,h} |

# Ignore language includes
grep -v "<stdint.h>\|<stddef.h>" |

# Isolate file, and it's dependency
awk -F':' '{print $1" "$3}' | awk '{print $1" "$3}' |
  awk -F'.' '{print $1" "$2}' | awk '{print $1" "$3}' |
  awk -F'"' '{print $1 $2}' |

# print diagram source
awk 'BEGIN{
}
{
  if($1 != $2)
  {
    print "  \""$1"\" -> \""$2"\""
  }
}
END{
}' |
sort | uniq &&

echo "}" &&

exit 0
# ============================================================================ #

