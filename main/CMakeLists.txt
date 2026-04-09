
# 1. 定义小智源码的绝对路径
set(XIAOZHI_MAIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../xiaozhi-esp32/main")

# 2. 收集我们企鹅机器人自己的源文件
set(PENGUIN_SOURCES
    "main.cpp"
    "hal/hal.cpp"
    "driver/SG90Servo/sg90Servo.cc"
    "board/m5stack_core_s3.cc"
    "board/cores3_audio_codec.cc"
    "board/my_display.cc"
    "ui/SmileAvatar.cc"
    # 你的 SmileAvatar 只有头文件，所以这里不用写
)

# 3. 收集小智大脑的核心源文件 (直接从 StackChan 的列表里剔除我们不需要的 display 和 led)
set(XIAOZHI_SOURCES 
    "audio/audio_codec.cc"
    "audio/audio_service.cc"
    # "audio/codecs/no_audio_codec.cc"
    # ✅ 换成 S3 专属的 AFE 处理器和双唤醒引擎：
    "audio/processors/afe_audio_processor.cc"
    "audio/wake_words/afe_wake_word.cc"
    "audio/wake_words/custom_wake_word.cc"
    "audio/codecs/box_audio_codec.cc"
    "audio/codecs/es8311_audio_codec.cc"
    "audio/codecs/es8374_audio_codec.cc"
    "audio/codecs/es8388_audio_codec.cc"
    "audio/codecs/es8389_audio_codec.cc"
    "audio/codecs/dummy_audio_codec.cc"
    "audio/processors/audio_debugger.cc"
    "led/single_led.cc"
    "led/circular_strip.cc"
    "led/gpio_led.cc"
    "display/display.cc"
    "display/lcd_display.cc"
    "display/oled_display.cc"
    "display/lvgl_display/lvgl_display.cc"
    "display/emote_display.cc"
    "display/lvgl_display/emoji_collection.cc"
    "display/lvgl_display/lvgl_theme.cc"
    "display/lvgl_display/lvgl_font.cc"
    "display/lvgl_display/lvgl_image.cc"
    "display/lvgl_display/gif/lvgl_gif.cc"
    "display/lvgl_display/gif/gifdec.c"
    "display/lvgl_display/jpg/image_to_jpeg.cpp"
    "protocols/protocol.cc"
    "protocols/mqtt_protocol.cc"
    "protocols/websocket_protocol.cc"
    "mcp_server.cc"
    "system_info.cc"
    "application.cc"
    "ota.cc"
    "settings.cc"
    "device_state_event.cc"
    "assets.cc"
    )
# 🌟 核心魔法：将小智的相对路径加上绝对路径前缀，越过目录墙抓取文件！
list(TRANSFORM XIAOZHI_SOURCES PREPEND "${XIAOZHI_MAIN_DIR}/")

# 4. 收集所有的头文件搜索路径
set(MY_INCLUDE_DIRS 
    "." 
    "ui" 
    "board"
    "hal"
    "driver/SG90Servo"
)
set(XIAOZHI_INCLUDE_DIRS "." "display" "display/lvgl_display" "display/lvgl_display/jpg" "audio" "protocols")

list(TRANSFORM XIAOZHI_INCLUDE_DIRS PREPEND "${XIAOZHI_MAIN_DIR}/")

# Add board common files
file(GLOB BOARD_COMMON_SOURCES ${XIAOZHI_MAIN_DIR}/boards/common/*.cc)
list(APPEND XIAOZHI_SOURCES ${BOARD_COMMON_SOURCES} )
list(APPEND XIAOZHI_INCLUDE_DIRS ${XIAOZHI_MAIN_DIR}/boards/common )


# ==========================================
# 🌟 补坑 1：配置多语言与收集 OGG 音频提示音
# ==========================================
set(LANG_DIR "zh-CN") # 设置默认语言为中文
set(LANG_JSON "${XIAOZHI_MAIN_DIR}/assets/locales/${LANG_DIR}/language.json")
set(LANG_HEADER "${XIAOZHI_MAIN_DIR}/assets/lang_config.h")

# 自动扫描并收集该语言下的所有提示音文件
file(GLOB LANG_SOUNDS ${XIAOZHI_MAIN_DIR}/assets/locales/${LANG_DIR}/*.ogg)
file(GLOB COMMON_SOUNDS ${XIAOZHI_MAIN_DIR}/assets/common/*.ogg ${XIAOZHI_MAIN_DIR}/assets/sfx/*.ogg)

# 5. 合并并注册为唯一的 main 组件！
idf_component_register(
    SRCS ${PENGUIN_SOURCES} ${XIAOZHI_SOURCES}
    INCLUDE_DIRS ${MY_INCLUDE_DIRS} ${XIAOZHI_INCLUDE_DIRS}
    
    EMBED_FILES ${LANG_SOUNDS} ${COMMON_SOUNDS} 
    WHOLE_ARCHIVE
)

# ==========================================
# 🌟 终极护盾：防止 CMake 扫描依赖时执行高级命令导致崩溃
# ==========================================
if(NOT CMAKE_SCRIPT_MODE_FILE)

    # 1. 附加宏定义（外层加上标准的双引号，彻底消除 Syntax Warning）
    target_compile_definitions(${COMPONENT_LIB} PRIVATE 
        "BOARD_TYPE=\"m5stack-core-s3\""
        "BOARD_NAME=\"m5stack-core-s3\""
    )
    
    # 2. 强制要求 C++14 或更高标准
    set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14)

    # 3. 自动生成多语言头文件（路径用双引号包裹，防止遇到带空格的文件夹）
    add_custom_command(
        OUTPUT "${LANG_HEADER}"
        COMMAND python "${XIAOZHI_MAIN_DIR}/../scripts/gen_lang.py"
                --language "${LANG_DIR}"
                --output "${LANG_HEADER}"
        DEPENDS "${LANG_JSON}" "${XIAOZHI_MAIN_DIR}/../scripts/gen_lang.py"
        COMMENT "Generating language config for ${LANG_DIR}..."
    )

    # 4. 创建前置编译目标，强制主程序等待头文件生成
    add_custom_target(lang_header ALL DEPENDS "${LANG_HEADER}")
    add_dependencies(${COMPONENT_LIB} lang_header)

endif()