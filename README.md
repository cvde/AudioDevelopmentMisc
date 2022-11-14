# MidiFileParser
This is not a full-fledged MIDI library, but a small C++ header for parsing MIDI files. If you need to write your own MIDI parser, this should be a good starting point.

At the moment only MIDI types 0 and 1 are supported and you need at least C++11.

## Usage

``` cpp
#include <iostream>
#include "MidiFileParser.h"

int main()
{
    MidiFileParser testFile;

    try
    {
        testFile.parse("test.mid");
    }
    catch (const MidiFileParserException& e)
    {
        std::cout << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "MIDI type: " << testFile.midiType << "\n";
    std::cout << "Number of tracks: " << testFile.numberOfTracks << "\n";
    std::cout << "Ticks per quarter note: " << testFile.ticksPerQuarterNote << "\n";

    // std::vector containing the note on/off events: testFile.midiEvents
    std::cout << "Total amount of note on/off events: " << testFile.midiEvents.size() << "\n";

    for (const MidiFileParserMidiEvent& midiEvent: testFile.midiEvents)
    {
        std::cout << "=======================\n";
        std::cout << "track: " << midiEvent.track << "\n";
        std::cout << "channel: " << midiEvent.channel << "\n";
        std::cout << "tickAbsolute: " << midiEvent.tickAbsolute << "\n";
        std::cout << "tickDelta: " << midiEvent.tickDelta << "\n";
        std::cout << "noteOn: " << midiEvent.noteOn << "\n";
        std::cout << "key: " << midiEvent.key << "\n";
        std::cout << "velocity: " << midiEvent.velocity << "\n";
    }
}
```

## License
MidiFileParser is free software and distributed under the [MIT License](LICENSE).
