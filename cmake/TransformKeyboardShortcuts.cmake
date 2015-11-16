FILE(READ "${INPUT}" HELP_CONTENTS)
STRING(REGEX REPLACE "#menu\\('([^']+)'\\)" "<script>print_menu_item(\"\\1\");</script>" HELP_CONTENTS "${HELP_CONTENTS}")
STRING(REGEX REPLACE "#action\\('([^']+)'\\)" "<script>print_action(\"\\1\");</script>" HELP_CONTENTS "${HELP_CONTENTS}")
FILE(WRITE "${OUTPUT}" "${HELP_CONTENTS}")