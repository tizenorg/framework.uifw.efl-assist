INSTALL(
        DIRECTORY ./ DESTINATION include/${LIB_NAME}
        FILES_MATCHING
        PATTERN "*_private.h" EXCLUDE
        PATTERN "*.h"
        )
