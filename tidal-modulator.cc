#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/cosine_oscillator.h"
#include "stmlib/utils/random.h"
#include "tides/generator.h"
#include "utils/float_math.h"
#include "utils/fixed_math.h"

uint16_t p_values[6] = {0};
float shape = 0, shiftshape = 0, shape_lfo = 0;
bool gate = false, previous_gate = false;

enum LfoTarget {
    LfoTargetShape,
    LfoTargetShiftShape,
    LfoTargetParam1,
    LfoTargetParam2,
    LfoTargetPitch,
    LfoTargetAmplitude
};

inline float get_lfo_value(enum LfoTarget target) {
    return (p_values[k_user_osc_param_id3] == target ? shape_lfo : 0.0f);
}

inline float get_shape() {
    return clip01f(shape + get_lfo_value(LfoTargetShape));
}
inline float get_shift_shape() {
    return clip01f(shiftshape + get_lfo_value(LfoTargetShiftShape));
}
inline float get_param_id1() {
    return clip01f((p_values[k_user_osc_param_id1] * 0.01f) + get_lfo_value(LfoTargetParam1));
}

tides::Generator generator;

void OSC_INIT(uint32_t platform, uint32_t api)
{
    generator.Init();
    generator.set_range(tides::GENERATOR_RANGE_HIGH);
    generator.set_mode(tides::GENERATOR_MODE_LOOPING);
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
    
}
void OSC_NOTEOFF(const user_osc_param_t * const params)
{
    
}

void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
    shape_lfo = q31_to_f32(params->shape_lfo);
    
//     float note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);
//     note += (get_lfo_value(LfoTargetPitch) * 0.5);
//     int16_t pitch_ = note * (2 << 7);

    int16_t pitch_ = params->pitch;
    int16_t shape_ = get_shape() * (2 << 15);
    int16_t slope_ = get_shift_shape() * (2 << 15);
    int16_t smooth_ = get_param_id1() * (2 << 15);
    
    if (generator.writable_block()) {
        generator.set_pitch(pitch_);
        generator.set_shape(shape_);
        generator.set_slope(slope_);
        generator.set_smoothness(smooth_);
        generator.Process();
    }
    
    for(uint32_t i = 0; i < frames; ++i) {
        const tides::GeneratorSample& sample = generator.Process(0);
        yn[i] = sample.bipolar * (2 << 16);
    }
    
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
    switch (index)
    {
        case k_user_osc_param_id1:
        case k_user_osc_param_id2:
        case k_user_osc_param_id3:
        case k_user_osc_param_id4:
        case k_user_osc_param_id5:
        case k_user_osc_param_id6:
            p_values[index] = value;
            break;
            
        case k_user_osc_param_shape:
            shape = param_val_to_f32(value);
            break;
            
        case k_user_osc_param_shiftshape:
            shiftshape = param_val_to_f32(value);
            break;
            
        default:
            break;
    }
}
