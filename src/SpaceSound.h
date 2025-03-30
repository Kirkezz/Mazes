#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <numbers>
#include <vector>

using namespace std::numbers;

/** \brief A helper class used by `Space` for calculations related to sonification.
  * Based on and adapted from the `SortSound.cpp` file of the [The Sound of Sorting](https://github.com/bingmann/sound-of-sorting) project */
class SpaceSound {
  public:
    unsigned sampleRate;

    size_t maxOscillatorsN = 0;
    SpaceSound(size_t maxOscillatorsN) : maxOscillatorsN(maxOscillatorsN) {}

    static inline double oscCompletionThreshold = 0.25;
    int curTick = 0;
    /// add an oscillator to the list (reuse finished ones)
    void addOscillator(double freq, size_t p, size_t pstart, size_t duration) {
        size_t oldest = 0, toldest = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < oscArr.size(); ++i) {
            if (oscArr[i].is_done(p) <= oscCompletionThreshold) {
                oscArr[i] = Oscillator(freq, pstart, duration, sampleRate);
                return;
            }

            if (oscArr[i].tstart < toldest) {
                oldest = i;
                toldest = oscArr[i].tstart;
            }
        }

        if (oscArr.size() < maxOscillatorsN) {
            oscArr.emplace_back(freq, pstart, duration, sampleRate);
        } else {
            oscArr[oldest] = Oscillator(freq, pstart, duration, sampleRate);
        }
    }

    static inline double oscAliveThreshold = 0.1;
    static inline double volumeSmoothingFactor = 0.9;
    double oldvol = 1.0;
    void wave(double *samples, int N, int notEnoughOscillators) {
        std::vector<double> samplesInternal(N, 0.0);
        size_t wavecount = 0;

        std::sort(oscArr.begin(), oscArr.end(), [](const auto &a, const auto &b) { return a.tstart > b.tstart; });
        for (auto &i : oscArr) {
            if (notEnoughOscillators > 0 || i.is_done(curTick) >= oscAliveThreshold) {
                i.mix(samplesInternal, curTick);
                ++wavecount;
                --notEnoughOscillators;
            }
        }

        // scale output with number of waves mixed
        if (wavecount) {
            // count maximum wave amplitude
            double vol = *std::max_element(samplesInternal.begin(), samplesInternal.end());

            if (vol > oldvol) {
                // immediately ramp down volume
            } else {
                // but slowly ramp up volume
                vol = volumeSmoothingFactor * oldvol;
            }

            // convert waveform to samples, with ramping of volume
            for (size_t i = 0; i < N; ++i) {
                int32_t v = 24000.0 * samplesInternal[i] / (oldvol + (vol - oldvol) * (i / (double) N));

                if (v > 32200) { v = 32200; }
                if (v < -32200) { v = -32200; }

                samples[i] = v / 32767.;
            }

            oldvol = vol;
        }

        // advance sample timestamp
        curTick += N;
    }

    // private:
    struct Oscillator {
        /// construct new oscillator
        Oscillator(double freq, size_t tstart, size_t duration, unsigned sampleRate)
            : freq(freq), tstart(tstart), tend(tstart + duration), duration(duration), sampleRate(sampleRate) {}

        // *** Wave Forms

        /// simple sine wave
        static double wave_sin(double x) { return sin(x * 2 * pi); }
        /// sin^3 wave
        static double wave_sin3(double x) {
            double s = sin(x * 2 * pi);
            return s * s * s;
        }
        /// triangle wave
        static double wave_triangle(double x) {
            x = fmod(x, 1.0);

            if (x <= 0.25) return 4.0 * x;
            if (x <= 0.75) return 2.0 - 4.0 * x;
            return 4.0 * x - 4.0;
        }
        static constexpr std::array waveFns = {wave_sin, wave_sin3, wave_triangle};
        static inline int currentWave = 2;

        // *** Envelope settings
        static inline double attack = 0.025; // percentage of duration
        static inline double decay = 0.1;    // percentage of duration
        static inline double sustain = 0.9;  // percentage of amplitude
        static inline double release = 0.3;  // percentage of duration

        /// envelope applied to wave (uses ADSR)
        double envelope(size_t i) const {
            double x = (double) i / duration;
            if (x > 1.0) x = 1.0;

            // *** ADSR envelope

            if (x < attack) return 1.0 / attack * x;

            if (x < attack + decay) return 1.0 - (x - attack) / decay * (1.0 - sustain);

            if (x < 1.0 - release) return sustain;

            return sustain / release * (1.0 - x);
        }

        // *** Generate Wave and Mix
        /// mix in the output of this oscillator on the wave output
        void mix(std::vector<double> &data, size_t p) const {
            for (int i = 0; i < data.size(); ++i) {
                if (p + i < tstart) continue;

                size_t trel = (p + i - tstart);

                double triangleComponent = is_done(p) * envelope(trel) * waveFns[currentWave]((double) trel / sampleRate * freq);
                double sineComponent = is_done(p) * envelope(trel) * wave_sin((double) trel / sampleRate * freq);

                // Adjust amplitudes (e.g., 0.7 triangle, 0.3 sine)
                data[i] += 0.7 * triangleComponent + 0.3 * sineComponent;
            }
        }

        /// (return value < 1.0) if (p > tend)
        double is_done(size_t p) const { return (p < tend || p == tstart) ? 1.0 : (double(tend - tstart) / (p - tstart)); }

        /// frequency of generated wave
        double freq;
        /// start and end of wave in sample time
        size_t tstart, tend;
        /// duration of oscillation note
        size_t duration;
        unsigned sampleRate;
    };
    /// array of oscillators
    std::vector<Oscillator> oscArr;
};
