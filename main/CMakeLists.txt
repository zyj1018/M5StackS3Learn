# 显式列出所有源文件（告别 GLOB 缓存 Bug）
set(src_files 
    "main.cpp" 
    "board/cores3_audio_codec.cc"
    # 如果你 main 目录下还有其他的 .cpp/.c 文件，请依次添加在这里
    # 例如："ui/SmileAvatar.cpp"
)

# 注册 main 组件
idf_component_register(
    SRCS ${src_files}
    INCLUDE_DIRS "." "ui" "network" "board"
    # 注意这里：把 esp_audio_codec 修正为了官方正确的 esp_codec_dev
    REQUIRES lvgl smooth_ui_toolkit esp_lcd_touch esp_wifi nvs_flash esp_codec_dev
)

# 强制要求 C++14 或更高标准
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14)