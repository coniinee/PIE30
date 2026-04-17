#pragma once

#include <JuceHeader.h>

inline juce::String u8s (const char* text)
{
    return juce::String::fromUTF8 (text);
}
