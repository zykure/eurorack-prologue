#include "userosc.h"
#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/limiter.h"
#include "stmlib/dsp/cosine_oscillator.h"
#include "plaits/dsp/engine/speech_engine.h"
#include "plaits/dsp/speech/naive_speech_synth.h"
#include "plaits/dsp/speech/sam_speech_synth.h"
#include "utils/float_math.h"
#include "utils/fixed_math.h"

uint16_t p_values[6] = {0};
float shape = 0, shiftshape = 0, shape_lfo = 0, lfo2 = 0, mix = 0, blend = 0;
bool gate = false, previous_gate = false;

plaits::EngineParameters parameters = {
    .trigger = plaits::TRIGGER_UNPATCHED,
    .note = 0,
    .timbre = 0,
    .morph = 0,
    .harmonics = 0,
    .accent = 0
};

stmlib::CosineOscillator lfo;

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

plaits::NaiveSpeechSynth naive_speech_synth_;
plaits::SAMSpeechSynth sam_speech_synth_;

void OSC_INIT(uint32_t platform, uint32_t api)
{
    lfo.InitApproximate(0);
    lfo.Start();
    
    sam_speech_synth_.Init();
    naive_speech_synth_.Init();
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
    static float out1[plaits::kMaxBlockSize], out2[plaits::kMaxBlockSize];
    static float aux1[plaits::kMaxBlockSize], aux2[plaits::kMaxBlockSize];

    shape_lfo = q31_to_f32(params->shape_lfo);
    lfo.InitApproximate(get_param_lfo2_frequency() / 600.f);
    lfo2 = (lfo.Next() - 0.5f) * 2.0f * get_param_lfo2_depth();
    
    parameters.note = ((float)(params->pitch >> 8)) + ((params->pitch & 0xFF) * k_note_mod_fscale);
    parameters.note += (get_lfo_value(LfoTargetPitch) * 0.5);
    
    if(gate && !previous_gate) {
        parameters.trigger = plaits::TRIGGER_RISING_EDGE;
    } else {
        parameters.trigger = plaits::TRIGGER_LOW;
    }
    previous_gate = gate;
    
    parameters.timbre = get_shift_shape();
    parameters.morph = get_shape();
    blend = get_param_id1();
    mix = get_param_id2();
    
    const float f0 = plaits::NoteToFrequency(parameters.note);
    
    naive_speech_synth_.Render(
        parameters.trigger == plaits::TRIGGER_RISING_EDGE,
        f0,
        parameters.morph,
        parameters.timbre,
        nullptr,
        aux1,
        out1,
        plaits::kMaxBlockSize);
    
    sam_speech_synth_.Render(
        parameters.trigger == plaits::TRIGGER_RISING_EDGE,
        f0,
        parameters.morph,
        parameters.timbre,
        aux2,
        out2,
        plaits::kMaxBlockSize);

    for(size_t i=0;i<plaits::kMaxBlockSize;i++) {
        float o = stmlib::Crossfade(out1[i], out2[i], blend);
        float a = stmlib::Crossfade(aux1[i], aux2[i], blend);
        yn[i] = f32_to_q31(stmlib::Crossfade(o, a, mix));
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
