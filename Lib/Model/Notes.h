//
// Created by Tibor Vass on 04.03.23.
//

#ifndef Notes_h
#define Notes_h

#include <cmath>
#include <json.hpp>
#include <vector>

#include "Constants.h"

enum PitchBend
{
    NoPitchBend,
    SinglePitchBend,
    MultiPitchBend
};

NLOHMANN_JSON_SERIALIZE_ENUM(PitchBend,
                             {
                                 {NoPitchBend, nullptr},
                                 {SinglePitchBend, "single"},
                                 {MultiPitchBend, "multi"},
                             })

class Notes
{
public:
    typedef struct Event
    {
        double start;
        double end;
        int pitch; // pitch is not in Hz, but in "MIDI note number"
        double amplitude;
        std::vector<int> bends;

        bool operator==(const struct Event&) const;
    } Event;

    typedef struct ConvertParams
    {
        /* Confidence threshold (0.05 to 0.95, More-Less notes) */
        float onsetThreshold = 0.3;
        /* Note segmentation (0.05 - 0.95, Split-Merge Notes) */
        float frameThreshold = 0.5;
        int minNoteLength = 11;
        bool inferOnsets = false;
        float maxFrequency = -1; // in Hz, -1 means unset
        float minFrequency = -1; // in Hz, -1 means unset
        bool melodiaTrick = false;
        enum PitchBend pitchBend = NoPitchBend;
        int energyThreshold = 11;
    } ConvertParams;

    // PG stands for posteriorgrams
    std::vector<Notes::Event> convert(const std::vector<std::vector<float>>& inNotesPG,
                                      const std::vector<std::vector<float>>& inOnsetsPG,
                                      const std::vector<std::vector<float>>& inContoursPG,
                                      ConvertParams inParams);

private:
    static inline double _modelFrameToTime(int frame)
    {
        // The following are compile-time computed consts only used here.
        // If they need to be used elsewhere, please move to Constants.h

        static constexpr int ANNOTATIONS_FPS = AUDIO_SAMPLE_RATE / FFT_HOP;
        // number of frames in the time-frequency representations we compute
        static constexpr int ANNOT_N_FRAMES = ANNOTATIONS_FPS * AUDIO_WINDOW_LENGTH;
        // number of samples in the (clipped) audio that we use as input to the models
        static constexpr int AUDIO_N_SAMPLES =
            AUDIO_SAMPLE_RATE * AUDIO_WINDOW_LENGTH - FFT_HOP;
        // magic from Basic Pitch
        static constexpr double WINDOW_OFFSET =
            (double) FFT_HOP / AUDIO_SAMPLE_RATE
                * (ANNOT_N_FRAMES - AUDIO_N_SAMPLES / (double) FFT_HOP)
            + 0.0018;

        return (frame * FFT_HOP) / (double) (AUDIO_SAMPLE_RATE) -WINDOW_OFFSET
               * (frame / ANNOT_N_FRAMES);
    }

    static inline int _hzToFreqIdx(float hz)
    {
        return (int) std::round(12.0 * (std::log2(hz) - std::log2(440.0)) + 69.0
                                - MIDI_OFFSET);
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    Notes::Event, start, end, pitch, amplitude, bends)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Notes::ConvertParams,
                                                onsetThreshold,
                                                frameThreshold,
                                                minNoteLength,
                                                inferOnsets,
                                                maxFrequency,
                                                minFrequency,
                                                melodiaTrick,
                                                pitchBend,
                                                energyThreshold)

#endif // Notes_h
