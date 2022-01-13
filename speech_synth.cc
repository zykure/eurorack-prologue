#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/limiter.h"
#include "plaits/dsp/engine/speech_engine.h"
#include "plaits/dsp/speech/naive_speech_synth.h"
#include "plaits/dsp/speech/sam_speech_synth.h"
#include "utils/float_math.h"
#include "utils/fixed_math.h"

uint16_t p_values[6] = {0};
float shape = 0, shiftshape = 0, shape_lfo = 0, mix = 0;
bool gate = false, previous_gate = false;

plaits::EngineParameters parameters = {
    .trigger = plaits::TRIGGER_UNPATCHED,
    .note = 0,
    .timbre = 0,
    .morph = 0,
    .harmonics = 0,
    .accent = 0
};

enum LfoTarget {
    LfoTargetShape,
    LfoTargetShiftShape,
    LfoTargetParam1,
    LfoTargetParam2,
    LfoTargetParam3,
    LfoTargetAmplitude,
};

inline float get_lfo_value(enum LfoTarget target) {
    return (p_values[k_user_osc_param_id4] == target ? shape_lfo : 0.0f);
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
inline float get_param_id2() {
    return clip01f((p_values[k_user_osc_param_id2] * 0.01f) + get_lfo_value(LfoTargetParam2));
}
inline float get_param_id3() {
    return clip01f((p_values[k_user_osc_param_id3] * 0.005f) + 0.5 + get_lfo_value(LfoTargetParam3));
}

#if defined(USE_LIMITER)
stmlib::Limiter limiter_;
#endif

plaits::NaiveSpeechSynth naive_speech_synth_;
plaits::SAMSpeechSynth sam_speech_synth_;

float* temp_buffer_[2];

void OSC_INIT(uint32_t platform, uint32_t api)
{
    stmlib::BufferAllocator allocator;
    temp_buffer_[0] = allocator.Allocate<float>(plaits::kMaxBlockSize);
    temp_buffer_[1] = allocator.Allocate<float>(plaits::kMaxBlockSize);
    
    sam_speech_synth_.Init();
    naive_speech_synth_.Init();

    #if defined(USE_LIMITER)
    limiter_.Init();
    #endif

    p_values[0] = 100;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
    gate = true;
}
void OSC_NOTEOFF(const user_osc_param_t * const params)
{
    gate = false;
}

void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
    static float out[plaits::kMaxBlockSize], aux[plaits::kMaxBlockSize];
    float* temp_buffer_[2];

    shape_lfo = q31_to_f32(params->shape_lfo);
    
    parameters.note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);
    //parameters.note += (get_lfo_value(LfoTargetPitch) * 0.5);
    
    if(gate && !previous_gate) {
        parameters.trigger = plaits::TRIGGER_RISING_EDGE;
    } else {
        parameters.trigger = plaits::TRIGGER_LOW;
    }
    previous_gate = gate;
    
    parameters.harmonics = get_shift_shape();
    parameters.timbre = get_param_id1();
    parameters.morph = get_shape();
    parameters.accent = get_param_id2();
    mix = get_param_id3();
    
    const float f0 = plaits::NoteToFrequency(parameters.note);
    
    naive_speech_synth_.Render(
        parameters.trigger == plaits::TRIGGER_RISING_EDGE,
        f0,
        parameters.morph,
        parameters.timbre,
        temp_buffer_[0],
        aux,
        out,
        plaits::kMaxBlockSize);
    
    sam_speech_synth_.Render(
        parameters.trigger == plaits::TRIGGER_RISING_EDGE,
        f0,
        parameters.harmonics,
        parameters.accent,
        temp_buffer_[0],
        temp_buffer_[1],
        plaits::kMaxBlockSize);
    
    for (size_t i = 0; i < plaits::kMaxBlockSize; ++i) {
        aux[i] += (temp_buffer_[0][i] - aux[i]) * mix;
        out[i] += (temp_buffer_[1][i] - out[i]) * mix;
    }
    
    #if defined(USE_LIMITER)
    limiter_.Process(1.0, out, plaits::kMaxBlockSize);
    #endif
    for(size_t i=0;i<plaits::kMaxBlockSize;i++) {
        yn[i] = f32_to_q31(out[i]);
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
