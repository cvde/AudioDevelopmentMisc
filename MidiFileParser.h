#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

struct MidiFileParserMidiEvent
{
    MidiFileParserMidiEvent(int track, int channel, bool noteOn, int tickDelta, int tickAbsolute, int key, int velocity) 
        : track(track)
        , channel(channel)
        , noteOn(noteOn)
        , tickDelta(tickDelta)
        , tickAbsolute(tickAbsolute)
        , key(key)
        , velocity(velocity)
    {}

    int track;
    int channel;
    bool noteOn;
    int tickDelta;
    int tickAbsolute;
    int key;
    int velocity;

    bool operator<(const MidiFileParserMidiEvent& midiEvent) const
    {
        return tickAbsolute < midiEvent.tickAbsolute;
    }
};

class MidiFileParserException : public std::exception
{
public:
    MidiFileParserException(const std::string& message)
        : message(message)
    {}
    const char* what() const noexcept
    {
        return message.c_str();
    }

private:
    std::string message;
};

class MidiFileParser
{
public:
    MidiFileParser() = default;
    MidiFileParser(const std::string& fileName)
    {
        parse(fileName);
    }

    int midiType;
    int numberOfTracks;
    int ticksPerQuarterNote;
    std::vector<MidiFileParserMidiEvent> midiEvents;

    void parse(const std::string& fileName)
    {
        reset();

        std::ifstream midiFile;
        open(midiFile, fileName);

        //
        // MIDI header
        //

        std::string headerId = readString(midiFile, 4);
        if (headerId != "MThd")
            throw(MidiFileParserException("MIDI header not compliant with standard"));

        auto headerSize = readFixedLengthValue<uint32_t>(midiFile);
        if (headerSize != 6)
            throw(MidiFileParserException("MIDI header size not compliant with standard"));

        auto midiType = readFixedLengthValue<uint16_t>(midiFile);
        this->midiType = midiType;
        // 0 -> single track, 1 -> multiple tracks, 2 -> not supported atm
        if (midiType > 1)
            throw(MidiFileParserException("MIDI type not supported"));

        auto numberOfTracks = readFixedLengthValue<uint16_t>(midiFile);
        this->numberOfTracks = numberOfTracks;
        if (numberOfTracks == 0)
            throw(MidiFileParserException("No MIDI tracks found"));
        if (midiType == 0 && numberOfTracks != 1)
            throw(MidiFileParserException("MIDI type 0 may only have one track"));

        auto ticksPerQuarterNote = readFixedLengthValue<uint16_t>(midiFile);
        this->ticksPerQuarterNote = ticksPerQuarterNote;

        //
        // MIDI tracks
        //

        for (int track = 0; track < numberOfTracks; ++track)
        {
            //
            // MIDI track header
            //

            std::string trackId = readString(midiFile, 4);
            if (trackId != "MTrk")
                throw(MidiFileParserException("MIDI track header not compliant with standard"));

            // I don't use trackSize because every track ends with META_EVENT_END_OF_TRACK
            auto trackSize = readFixedLengthValue<uint32_t>(midiFile);

            //
            // MIDI track events
            //

            uint8_t previousMidiEvent = 0;
            uint32_t tickAbsolute = 0;

            while (true)
            {
                uint32_t tickDelta = readVariableLengthValue(midiFile);
                tickAbsolute += tickDelta;

                auto midiEvent = readFixedLengthValue<uint8_t>(midiFile);

                // check for "running status" feature
                if (midiEvent <= MIDI_DATA)
                {
                    if (previousMidiEvent == 0)
                    {
                        throw(MidiFileParserException("Running status feature used without previous MIDI event"));
                    }
                    else
                    {
                        // midiEvent contains MIDI data and no MIDI event, reuse the previous MIDI event
                        midiEvent = previousMidiEvent;
                        int numberOfBytes = -1 * int(sizeof(midiEvent));
                        rewind(midiFile, numberOfBytes); // go one step back to process the MIDI data below
                    }
                }
                else
                {
                    if (midiEvent < SYSEX_EVENT)
                    {
                        // running status only applies for the events MIDI_EVENT_NOTE_OFF to MIDI_EVENT_PITCH_BEND
                        previousMidiEvent = midiEvent;
                    }
                    else if (midiEvent < META_EVENT)
                    {
                        // reset running status for the events SYSEX_EVENT and SYSEX_EVENT_EOX
                        previousMidiEvent = 0;
                    }
                }

                // some events contain the channel - remove the channel to make the following if statement easier
                uint8_t midiEventWithoutChannel = midiEvent & 0xf0u; // the last 4 bits are the event

                if (midiEventWithoutChannel == MIDI_EVENT_NOTE_OFF || midiEventWithoutChannel == MIDI_EVENT_NOTE_ON)
                {
                    // if the "running status" feature is used, a MIDI_EVENT_NOTE_ON event with a velocity of 0 is acutally a MIDI_EVENT_NOTE_OFF event

                    // 2 additional bytes
                    uint8_t channel = (midiEvent & 0x0fu); // the first 4 bits are the channel
                    auto key = readFixedLengthValue<uint8_t>(midiFile);
                    auto velocity = readFixedLengthValue<uint8_t>(midiFile);
                    if (velocity > 0)
                    {
                        midiEvents.emplace_back(track, channel, true, tickDelta, tickAbsolute, key, velocity);
                    }
                    else
                    {
                        midiEvents.emplace_back(track, channel, false, tickDelta, tickAbsolute, key, velocity);
                    }
                }
                else if (midiEventWithoutChannel == MIDI_EVENT_POLYPHONIC_PRESSURE)
                {
                    // 2 additional bytes
                    // uint8_t channel = (midiEvent & 0x0fu);
                    // auto key = readFixedLengthValue<uint8_t>(midiFile);
                    // auto pressure = readFixedLengthValue<uint8_t>(midiFile);
                    skip(midiFile, 2);
                }
                else if (midiEventWithoutChannel == MIDI_EVENT_CONTROLLER)
                {
                    // 2 additional bytes
                    // uint8_t channel = (midiEvent & 0x0fu);
                    // auto controller = readFixedLengthValue<uint8_t>(midiFile);
                    // auto value = readFixedLengthValue<uint8_t>(midiFile);
                    skip(midiFile, 2);
                }
                else if (midiEventWithoutChannel == MIDI_EVENT_PROGRAMM_CHANGE)
                {
                    // 1 additional byte
                    // uint8_t channel = (midiEvent & 0x0fu);
                    // auto program = readFixedLengthValue<uint8_t>(midiFile);
                    skip(midiFile, 1);
                }
                else if (midiEventWithoutChannel == MIDI_EVENT_CHANNEL_PRESSURE)
                {
                    // 1 additional byte
                    // uint8_t channel = (midiEvent & 0x0fu);
                    // auto pressure = readFixedLengthValue<uint8_t>(midiFile);
                    skip(midiFile, 1);
                }
                else if (midiEventWithoutChannel == MIDI_EVENT_PITCH_BEND)
                {
                    // 2 additional bytes
                    // uint8_t channel = (midiEvent & 0x0fu);
                    // auto lsb = readFixedLengthValue<uint8_t>(midiFile);
                    // auto msb = readFixedLengthValue<uint8_t>(midiFile);
                    // uint16_t pitchAdjustment = (lsb & 0x7fu); // clear bit 8 and add remaining bits
                    // pitchAdjustment <<= 7; // shift by 7 bits to make room for msb
                    // pitchAdjustment += (msb & 0x7fu); // clear bit 8 and add remaining bits
                    skip(midiFile, 2);
                }
                else if (midiEvent == SYSEX_EVENT || midiEvent == SYSEX_EVENT_EOX)
                {
                    // variable length
                    auto length = readVariableLengthValue(midiFile);
                    skip(midiFile, length);
                }
                else if (midiEvent == META_EVENT)
                {
                    auto midiMetaEvent = readFixedLengthValue<uint8_t>(midiFile);

                    // some meta events have a fixed length and others have a variable length
                    // nevertheless, all meta events have a variable length field that specifies the length of the event
                    auto midiMetaEventLength = readVariableLengthValue(midiFile);

                    if (midiMetaEvent == META_EVENT_SEQUENCE_NUMBER)
                    {
                        // 2 additional bytes
                        if (midiMetaEventLength != 2)
                            throw(MidiFileParserException("Sequence number meta event not compliant with standard"));
                        // auto sequenceNumber = readFixedLengthValue<uint16_t>(midiFile);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_TEXT)
                    {
                        // variable length
                        // std::string text = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_COPYRIGHT_NOTICE)
                    {
                        // variable length
                        // std::string copyrightNotice = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_TRACK_NAME)
                    {
                        // variable length
                        // std::string trackName = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_INSTRUMENT_NAME)
                    {
                        // variable length
                        // std::string instrumentName = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_LYRICS)
                    {
                        // variable length
                        // std::string lyrics = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_MARKER)
                    {
                        // variable length
                        // std::string marker = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_CUE_POINT)
                    {
                        // variable length
                        // std::string cuePoint = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_PROGRAM_NAME)
                    {
                        // variable length
                        // std::string programName = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_DEVICE_NAME)
                    {
                        // variable length
                        // std::string deviceName = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_MIDI_CHANNEL_PREFIX)
                    {
                        // 1 addtional byte
                        if (midiMetaEventLength != 1)
                            throw(MidiFileParserException("Channel prefix meta event not compliant with standard"));
                        // auto channelPrefix = readFixedLengthValue<uint8_t>(midiFile);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_MIDI_PORT)
                    {
                        // 1 addtional byte
                        if (midiMetaEventLength != 1)
                            throw(MidiFileParserException("MIDI port meta event not compliant with standard"));
                        // auto port = readFixedLengthValue<uint8_t>(midiFile);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_END_OF_TRACK)
                    {
                        // 0 additional bytes
                        if (midiMetaEventLength != 0)
                            throw(MidiFileParserException("End of track meta event not compliant with standard"));
                        break;
                    }
                    else if (midiMetaEvent == META_EVENT_TEMPO)
                    {
                        // 3 additional bytes
                        if (midiMetaEventLength != 3)
                            throw(MidiFileParserException("Tempo meta event not compliant with standard"));
                        // uint32_t tempoMicrosecondsPerQuarterNote = (readFixedLengthValue<uint8_t>(midiFile) << 16 ) | (readFixedLengthValue<uint8_t>(midiFile) << 8 ) | (readFixedLengthValue<uint8_t>(midiFile));
                        // uint32_t tempoBPM = 60000000 / tempoMicrosecondsPerQuarterNote;
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_SMPTE_OFFSET)
                    {
                        // 5 additional bytes
                        if (midiMetaEventLength != 5)
                            throw(MidiFileParserException("SMPTE offset meta event not compliant with standard"));
                        // auto hoursAndFrameRate = readFixedLengthValue<uint8_t>(midiFile);
                        // uint8_t hours = (hoursAndFrameRate & 0x1fu); // get first 5 bits
                        // uint8_t frameRate = (hoursAndFrameRate & 0x60u) >> 5; // get bit 6 and 7 and shift by 5
                        // switch (frameRate)
                        // {
                        //     case 0x00u:
                        //         // "24 fps"
                        //         break;
                        //     case 0x01u:
                        //         // "25 fps"
                        //         break;
                        //     case 0x02u:
                        //         // "30 fps (drop frame)"
                        //         break;
                        //     case 0x03u:
                        //         // "30 fps (non-drop frame)"
                        //         break;
                        // }
                        // auto minutes = readFixedLengthValue<uint8_t>(midiFile);
                        // auto seconds = readFixedLengthValue<uint8_t>(midiFile);
                        // auto frames = readFixedLengthValue<uint8_t>(midiFile);
                        // auto fractionalFrames = readFixedLengthValue<uint8_t>(midiFile);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_TIME_SIGNATURE)
                    {
                        // 4 additional bytes
                        if (midiMetaEventLength != 4)
                            throw(MidiFileParserException("Time signature meta event not compliant with standard"));
                        // auto numerator = readFixedLengthValue<uint8_t>(midiFile);
                        // auto denominator = std::pow(2, readFixedLengthValue<uint8_t>(midiFile));
                        // auto midiClocksBetweenMetronomeClicks = readFixedLengthValue<uint8_t>(midiFile);
                        // auto numberOf32NotesPerQuarterNote = readFixedLengthValue<uint8_t>(midiFile);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_KEY_SIGNATURE)
                    {
                        // 2 additional bytes
                        if (midiMetaEventLength != 2)
                            throw(MidiFileParserException("Key signature meta event not compliant with standard"));
                        // auto numberOfSharpsOrFlats = readFixedLengthValue<uint8_t>(midiFile); // values between -7 and 7, negative -> number of flats, positive -> number of sharps
                        // auto scale = readFixedLengthValue<uint8_t>(midiFile); // 0 -> major, 1 -> minor
                        skip(midiFile, midiMetaEventLength);
                    }
                    else if (midiMetaEvent == META_EVENT_SEQUENCER_SPECIFIC)
                    {
                        // variable length
                        // std::string sequencerSpecific = readString(midiFile, midiMetaEventLength);
                        skip(midiFile, midiMetaEventLength);
                    }
                    else
                    {
                        throw(MidiFileParserException("Unknown MIDI meta event found"));
                    }
                }
                else
                {
                    throw(MidiFileParserException("Unknown MIDI event found"));
                }
            }
        }
        // make sure all events are in correct chronological order
        std::sort(midiEvents.begin(), midiEvents.end());
    }

private:
    void reset()
    {
        midiType = 0;
        numberOfTracks = 0;
        ticksPerQuarterNote = 0;
        midiEvents.clear();
        midiEvents.reserve(20000); // choose a reasonable value
    }

    void open(std::ifstream& midiFile, const std::string& fileName)
    {
        midiFile.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
        try
        {
            midiFile.open(fileName, std::ios::binary);
        }
        catch (const std::ifstream::failure& e)
        {
            throw(MidiFileParserException("MIDI file not found"));
        }
    }

    template <typename T>
    T readFixedLengthValue(std::ifstream& midiFile)
    {
        T result;
        try
        {
            midiFile.read((char*)&result, sizeof(result));
        }
        catch (const std::ifstream::failure& e)
        {
            throw(MidiFileParserException("Read error, file might have ended unexpectedly"));
        }

        // if we have more than one byte we need to convert from big endian to little endian
        if (sizeof(result) > 1)
        {
            swapEndian(result);
        }

        return result;
    }

    template <typename T>
    void swapEndian(T& value)
    {
        union
        {
            T value;
            std::array<std::uint8_t, sizeof(T)> raw;
        } source, destination;

        source.value = value;
        std::reverse_copy(source.raw.begin(), source.raw.end(), destination.raw.begin());
        value = destination.value;
    }

    uint32_t readVariableLengthValue(std::ifstream& midiFile)
    {
        uint32_t result = 0;
        int bytesRead = 0;

        while (true)
        {
            auto byte = readFixedLengthValue<uint8_t>(midiFile);

            // only the first 7 bits are relevant, bit 8 indicates if we need to read further
            if (byte > 0x7fu) // bit 8 equals to 1, which means we are not done yet
            {
                result += (byte & 0x7fu); // clear bit 8 and add remaining bits to result
                result <<= 7;             // shift by 7 bits to make room for the next round
            }
            else
            {
                return result + byte; // we can simply add byte here because bit 8 is always 0
            }
            if (++bytesRead == 4)
            {
                throw(MidiFileParserException("Variable length value not compliant with standard"));
            }
        }
    }

    std::string readString(std::ifstream& midiFile, int numberOfBytes)
    {
        std::string result;
        result.resize(numberOfBytes);

        try
        {
            midiFile.read(&result[0], numberOfBytes); // since C++11 a std::string's allocation is guaranteed to be continuous
        }
        catch (const std::ifstream::failure& e)
        {
            throw(MidiFileParserException("Read error, file might have ended unexpectedly"));
        }

        return result;
    }

    void skip(std::ifstream& midiFile, int numberOfBytes)
    {
        try
        {
            midiFile.ignore(numberOfBytes);
        }
        catch (const std::ifstream::failure& e)
        {
            throw(MidiFileParserException("Read error, file might have ended unexpectedly"));
        }
    }

    void rewind(std::ifstream& midiFile, int numberOfBytes)
    {
        try
        {
            midiFile.seekg(numberOfBytes, std::ios_base::cur);
        }
        catch (const std::ifstream::failure& e)
        {
            throw(MidiFileParserException("Read error"));
        }
    }

    // MIDI data
    static const uint8_t MIDI_DATA = 0x7fu; // 0x0 - 0x7f --> MIDI data is within this range

    // MIDI events
    static const uint8_t MIDI_EVENT_NOTE_OFF = 0x80u;            // 0x80 - 0x8f
    static const uint8_t MIDI_EVENT_NOTE_ON = 0x90u;             // 0x90 - 0x9f
    static const uint8_t MIDI_EVENT_POLYPHONIC_PRESSURE = 0xa0u; // 0xa0 - 0xaf
    static const uint8_t MIDI_EVENT_CONTROLLER = 0xb0u;          // 0xb0 - 0xbf
    static const uint8_t MIDI_EVENT_PROGRAMM_CHANGE = 0xc0u;     // 0xc0 - 0xcf
    static const uint8_t MIDI_EVENT_CHANNEL_PRESSURE = 0xd0u;    // 0xd0 - 0xdf
    static const uint8_t MIDI_EVENT_PITCH_BEND = 0xe0u;          // 0xe0 - 0xef
    static const uint8_t SYSEX_EVENT = 0xf0u;
    // 0xf1 - 0xf6 not valid within a MIDI file
    static const uint8_t SYSEX_EVENT_EOX = 0xf7u;
    // 0xf8 - 0xfe not valid within a MIDI file
    static const uint8_t META_EVENT = 0xffu;

    // MIDI meta events
    static const uint8_t META_EVENT_SEQUENCE_NUMBER = 0x00u;
    static const uint8_t META_EVENT_TEXT = 0x01u;
    static const uint8_t META_EVENT_COPYRIGHT_NOTICE = 0x02u;
    static const uint8_t META_EVENT_TRACK_NAME = 0x03u;
    static const uint8_t META_EVENT_INSTRUMENT_NAME = 0x04u;
    static const uint8_t META_EVENT_LYRICS = 0x05u;
    static const uint8_t META_EVENT_MARKER = 0x06u;
    static const uint8_t META_EVENT_CUE_POINT = 0x07u;
    static const uint8_t META_EVENT_PROGRAM_NAME = 0x08u;
    static const uint8_t META_EVENT_DEVICE_NAME = 0x09u;
    static const uint8_t META_EVENT_MIDI_CHANNEL_PREFIX = 0x20u;
    static const uint8_t META_EVENT_MIDI_PORT = 0x21u;
    static const uint8_t META_EVENT_END_OF_TRACK = 0x2fu;
    static const uint8_t META_EVENT_TEMPO = 0x51u;
    static const uint8_t META_EVENT_SMPTE_OFFSET = 0x54u;
    static const uint8_t META_EVENT_TIME_SIGNATURE = 0x58u;
    static const uint8_t META_EVENT_KEY_SIGNATURE = 0x59u;
    static const uint8_t META_EVENT_SEQUENCER_SPECIFIC = 0x7fu;
};
