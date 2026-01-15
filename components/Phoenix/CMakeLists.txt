idf_component_register(
    SRCS
        # "src/Phoenix.c"
        # "src/Gait.c"
        # "src/IK.c"
        "src/platform.c"
    INCLUDE_DIRS
        "inc"
    PRIV_REQUIRES
        esp_timer
        freertos
)