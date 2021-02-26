# luaopus

Lua bindings to [libopus](https://opus-codec.org/docs/).

MIT licensed (see file `LICENSE`).

Currently covers the encoding and decoding APIs, but not
the packetization and multistream APIs.

# Installation

Available on [luarocks](https://luarocks.org/modules/jprjr/luaopus)

```bash
luarocks install luaopus
```

Available on [AUR](https://aur.archlinux.org/pkgbase/lua-luaopus/)

## Building from source

You can build with luarocks or cmake.

# Table of Contents

* [Synopsis](#synopsis)
* [Decoder Functions](#decoder-functions)
  * [OpusDecoder](#opusdecoder)
  * [opus\_decoder\_init](#opus_decoder_init)
  * [opus\_decode](#opus_decode)
  * [opus\_decode\_float](#opus_decode_float)
  * [opus\_decoder\_ctl](#opus_decoder_ctl)
* [Encoder Functions](#encoder-functions)
  * [OpusEncoder](#opusencoder)
  * [opus\_encoder\_init](#opus_encoder_init)
  * [opus\_encode](#opus_encode)
  * [opus\_encode\_float](#opus_encode_float)
  * [opus\_encoder\_ctl](#opus_encoder_ctl)

# Synopsis

```lua

-- demo program that decodes Opus and writes out
-- a headerless PCM file
-- PCM file can be played with ffmpeg/ffplay, like:
-- ffmpeg -f s16le -ar 48000 -ac 2 -i file.pcm file.wav
-- ffplay -f s16le -ar 48000 -ac 2 file.pcm
--
-- requires luaogg for parsing an Ogg stream

local ogg = require'luaogg'
local opus = require'luaopus'

-- takes a signed integer and packs it into
-- a 16-bit, little-endian
local function pack_int16(v)
  -- convert to unsigned
  if v < 0 then
    v = v + 0x10000
  end

  return string.char(v % 256) .. string.char(math.floor(v / 256))
end

local function get_page(sync,file)
  local page, chunk
  while not page do
    page = sync:pageout()
    if page then break end
    chunk = file:read(4096)
    if not chunk then break end
    sync:buffer(chunk)
  end
  return page
end

local function get_packet(stream,sync,file)
  local packet, page
  while not packet do
    packet = stream:packetout()
    if packet then break end
    page = get_page(sync,file)
    if not page then break end
    if page.bos then
      stream:init(page.serialno)
    end
    stream:pagein(page)
  end
  return packet
end

if not arg[2] then
  print(string.format('Usage: %s input output',arg[0]))
  os.exit(1)
end


local sync = ogg.ogg_sync_state()
local stream = ogg.ogg_stream_state()
local decoder = opus.OpusDecoder()
local packet

local input = io.open(arg[1],'rb')
if not input then
  os.exit(1)
end

local output = io.open(arg[2],'wb')
if not output then
  os.exit(1)
end

sync:init()


while true do
  packet = get_packet(stream,sync,input)
  if not packet then break end
  if string.len(packet.packet) < 4 then break end
  -- skip over the OpusHead and OpusTags packets
  if string.sub(packet.packet,1,4) ~= 'Opus' then break end
end

-- we should really check for an OpusTags packet and
-- use that for samplerate and channels, this
-- is good enough
decoder:init(48000,2)

while packet do
  local samples = decoder:decode(packet.packet)
  for _,s in ipairs(samples) do
    output:write(pack_int16(s))
  end
  packet = get_packet(stream,sync,input)
end

input:close()
output:close()
```

# Decoder Functions

## OpusDecoder

**syntax:** `userdata decoder = opus.OpusDecoder()`

Returns a new decoder instance.

Instance has a metatable allowing for object-oriented usage.

* `decoder:init(samplerate, channels)` -> `opus.opus_decoder_init(decoder, samplerate, channels)`
* `decoder:decode(packet)` -> `opus.opus_decode(decoder, packet)`
* `decoder:decode_float(packet)` -> `opus.opus_decode_float(decoder, packet)`

## opus_decoder_init

**syntax:** `boolean success = opus.opus_decoder_init(userdata decoder, number samplerate, number channels)`

Initializes a decoder instance for the given `samplerate` and `channels`.

## opus_decode

**syntax:** `table samples = opus.opus_decode(userdata decoder, string packet)`

Decodes an Opus packet into a table of integer samples. Table is array-like
and a single dimension (stereo samples are interleaved).

## opus_decode_float


**syntax:** `table samples = opus.opus_decode_float(userdata decoder, string packet)`

Decodes an Opus packet into a table of float samples. Table is array-like
and a single dimension (stereo samples are interleaved).

## opus_decoder_ctl

All the CTL functions are implemented as individual functions. Take the name of the CTL macro, append it to `opus_decoder_ctl_`, transform it to lowercase. `SET` functions will return a `boolean true` for success.
`GET` functions will return the appropriate data type for the CTL.

Examples:

```lua
opus_decoder_ctl_reset_state(userdata decoder)
rate = opus_decoder_ctl_get_sample_rate(userdata decoder)
```

Is equivalent to the following in C:

```c
opus_decoder_ctl(decoder, OPUS_RESET_STATE);
opus_decoder_ctl(decoder, OPUS_GET_SAMPLE_RATE(&rate));
```


All the CTL functions are also available via the object-oriented interface:

```lua
decoder:reset_state()
rate = decoder:get_sample_rate()
```

# Encoder Functions

## OpusEncoder

**syntax:** `userdata encoder = opus.OpusEncoder()`

Returns a new encoder instance.

Instance has a metatable allowing for object-oriented usage.

* `encoder:init(samplerate, channels, application)` -> `opus.opus_encoder_init(encoder, samplerate, channels, application)`
* `encoder:encode(samples)` -> `opus.opus_encode(encoder, samples)`
* `encoder:encode_float(samples)` -> `opus.opus_encode_float(encoder, samples)`

## opus_encoder_init

**syntax:** `boolean success = opus.opus_encoder_init(userdata encoder, number samplerate, number channels, number application)`

Initializes a encoder instance for the given `samplerate`, `channels`, and `application` (one of: `OPUS_APPLICATION_VOIP`, `OPUS_APPLICATION_AUDIO`, `OPUS_APPLICATION_RESTRICTED_LOWDELAY`).

## opus_encode

**syntax:** `string packet = opus.opus_encode(userdata encoder, table samples)`

Encodes an array-like table of integer samples into an Opus packet.
Table is single-dimensional (stereo samples are interleaved).


## opus_encode_float

**syntax:** `string packet = opus.opus_encode_float(userdata encoder, table samples)`

Encodes an array-like table of float samples into an Opus packet.
Table is single-dimensional (stereo samples are interleaved).

## opus_encoder_ctl

All the CTL functions are implemented as individual functions. Take the name of the CTL macro, append it to `opus_encoder_ctl_`, transform it to lowercase. `SET` functions will return a `boolean true` for success.
`GET` functions will return the appropriate data type for the CTL.

Examples:

```lua
opus_encoder_ctl_reset_state(userdata encoder)
rate = opus_encoder_ctl_get_sample_rate(userdata encoder)
opus_encoder_ctl_set_bitrate(userdata encoder, number bitrate)
```

Is equivalent to the following in C:

```c
opus_encoder_ctl(encoder, OPUS_RESET_STATE);
opus_encoder_ctl(encoder, OPUS_GET_SAMPLE_RATE(&rate));
opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
```


All the CTL functions are also available via the object-oriented interface:

```lua
encoder:reset_state()
rate = encoder:get_sample_rate()
encoder:set_bitrate(bitrate)
```
