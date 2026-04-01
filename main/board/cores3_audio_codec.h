#pragma once

#include <driver/gpio.h>
#include <driver/i2s_std.h>
#include <driver/i2s_tdm.h>
#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>
#include <audio_codec_data_if.h>
#include <audio_codec_ctrl_if.h>
#include <audio_codec_if.h>

class CoreS3AudioCodec {
public:
    CoreS3AudioCodec(void* i2c_master_handle, int input_sample_rate, int output_sample_rate,
                     gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din,
                     uint8_t aw88298_addr, uint8_t es7210_addr, bool input_reference);
    ~CoreS3AudioCodec();

    void SetOutputVolume(int volume);
    void EnableInput(bool enable);
    void EnableOutput(bool enable);
    int Read(int16_t* dest, int samples);
    int Write(const int16_t* data, int samples);

private:
    void CreateDuplexChannels(gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din);

    // 从原父类平移过来的状态变量
    bool input_enabled_ = false;
    bool output_enabled_ = false;
    int output_volume_ = 70;

    // CoreS3 自身的配置变量
    bool duplex_;
    bool input_reference_;
    int input_channels_;
    int input_sample_rate_;
    int output_sample_rate_;
    int input_gain_;

    // I2S 及 Codec 底层句柄
    i2s_chan_handle_t tx_handle_ = nullptr;
    i2s_chan_handle_t rx_handle_ = nullptr;

    audio_codec_data_if_t* data_if_ = nullptr;
    audio_codec_ctrl_if_t* out_ctrl_if_ = nullptr;
    audio_codec_ctrl_if_t* in_ctrl_if_ = nullptr;
    audio_codec_gpio_if_t* gpio_if_ = nullptr;
    audio_codec_if_t* out_codec_if_ = nullptr;
    audio_codec_if_t* in_codec_if_ = nullptr;

    esp_codec_dev_handle_t output_dev_ = nullptr;
    esp_codec_dev_handle_t input_dev_ = nullptr;
};