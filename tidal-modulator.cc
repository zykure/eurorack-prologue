#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/cosine_oscillator.h"
#include "stmlib/utils/random.h"
#include "tides/generator.h"
#include "utils/float_math.h"
#include "utils/fixed_math.h"

uint16_t p_values[6] = {0};
float shape = 0, shiftshape = 0, shape_lfo = 0, lfo2 = 0, mix = 0;
bool gate = false, previous_gate = false;

enum LfoTarget {
    LfoTargetShape,
    LfoTargetShiftShape,
    LfoTargetParam1,
    LfoTargetParam2,
    LfoTargetPitch,
    LfoTargetAmplitude,
    LfoTargetLfo2Frequency,
    LfoTargetLfo2Depth
};

inline float get_lfo_value(enum LfoTarget target) {
    return (p_values[k_user_osc_param_id3] == target ? shape_lfo : 0.0f) +
    (p_values[k_user_osc_param_id6] == target ? lfo2 : 0.0f);
}

inline float get_shape() {
    return clip01f(shape + get_lfo_value(LfoTargetShape));
}
inline float get_shift_shape() {
    return clip01f(shiftshape + get_lfo_value(LfoTargetShiftShape));
}
inline float get_param_id1() {
    return clip01f((p_values[k_user_osc_param_id1] * 0.005f) + get_lfo_value(LfoTargetParam1));
}
inline float get_param_id2() {
    return clip01f((p_values[k_user_osc_param_id2] * 0.01f) + get_lfo_value(LfoTargetParam2));
}
inline float get_param_lfo2_frequency() {
    return clip01f((p_values[k_user_osc_param_id4] * 0.01f) + get_lfo_value(LfoTargetLfo2Frequency));
}
inline float get_param_lfo2_depth() {
    return clip01f((p_values[k_user_osc_param_id5] * 0.01f) + get_lfo_value(LfoTargetLfo2Depth));
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
    int16_t shape = get_shape() * (2 << 15);
    int16_t slope = get_shift_shape() * (2 << 15);
    int16_t smooth = get_param_id1() * (2 << 15);
    
    if (generator.writable_block()) {
        generator.set_pitch(params->pitch);
        generator.set_shape(shape);
        generator.set_slope(slope);
        generator.set_smoothness(smooth);
        generator.Process();
    }
    
    
}
