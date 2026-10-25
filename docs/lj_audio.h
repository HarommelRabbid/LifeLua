/**
 * @defgroup audio audio
 * @brief Playing music and sounds
 *
 * Supported formats: .MP3, .WAV, .OGG, .FLAC, .OPUS and tracker modules (.MOD, .XM, .IT, .S3M, ...).
 * The format is picked from the file extension. Any other file is played as raw 16-bit stereo PCM at 48000 Hz.
 *
 * Every function that takes an `audio` as its first parameter can also be called as a method:
 * @code
 * local music = audio.load("app0:music.ogg")
 * music:play(true) -- loop forever
 * while true do
 *     draw.text(10, 10, string.format("%.1f / %.1f s", music:elapsed(), music:duration()), color.new(255, 255, 255))
 *     draw.swapbuffers()
 * end
 * @endcode
 *
 * @note Only one audio can play at a time: playing another one replaces the current one.
 * @{
*/
/**
 * Loads an audio file
 * @param path Path to the file (e.g. `"app0:music.mp3"`)
 * @return The loaded audio. Raises an error if the file can't be opened.
*/
audio audioꓸload(string path) {}
/**
 * Starts playing an audio, from the beginning if it wasn't already playing
 *
 * Method form: `snd:play(loop)`
 * @param audio The audio
 * @param loop `true` to start over every time it ends *(optional, default: false)*
*/
nil audioꓸplay(audio audio, boolean loop) {}
/**
 * Stops the audio output completely. Use audioꓸpause() if you want to continue later.
 *
 * Method form: `snd:stop()`
 * @param audio The audio
*/
nil audioꓸstop(audio audio) {}
/**
 * Pauses or resumes an audio
 *
 * Method form: `snd:pause(pause)`
 * @param audio The audio
 * @param pause `true` to pause, `false` to resume
 * @par Example:
 * @code
 * if controls.pressed(SCE_CTRL_START) then
 *     music:pause(not music:paused())
 * end
 * @endcode
*/
nil audioꓸpause(audio audio, boolean pause) {}
/**
 * Checks if an audio is playing
 *
 * Method form: `snd:playing()`
 * @param audio The audio
 * @return `true` while the audio is playing (also while it's paused), `false` once it has finished
*/
boolean audioꓸplaying(audio audio) {}
/**
 * Checks if an audio is paused
 *
 * Method form: `snd:paused()`
 * @param audio The audio
 * @return `true` if paused
*/
boolean audioꓸpaused(audio audio) {}
/**
 * Jumps to a position in the audio
 *
 * Method form: `snd:seek(seconds)`
 * @param audio The audio
 * @param seconds Position to jump to, in seconds from the start
*/
nil audioꓸseek(audio audio, number seconds) {}
/**
 * Sets the volume of the audio
 *
 * Method form: `snd:volume(left, right)`
 * @param audio The audio
 * @param left Volume from 0 to 100 *(optional, default: 100)*
 * @param right Volume of the right speaker from 0 to 100 *(optional, default: same as left)*
 * @par Example:
 * @code
 * music:volume(50)      -- half volume
 * music:volume(100, 0)  -- left speaker only
 * @endcode
*/
nil audioꓸvolume(audio audio, number left, number right) {}
/**
 * Gets the total length of an audio
 *
 * Method form: `snd:duration()`
 * @param audio The audio
 * @return The length in seconds, or `nil` if it's unknown
*/
number audioꓸduration(audio audio) {}
/**
 * Gets how far an audio has played
 *
 * Method form: `snd:elapsed()`
 * @param audio The audio
 * @return The current position in seconds, or `nil` if it's unknown
*/
number audioꓸelapsed(audio audio) {}
/**
 * Gets the ID3v1 tag of an .MP3 file
 *
 * Method form: `snd:id3v1()`
 * @param audio The audio (.MP3 only)
 * @return `nil` if there is no ID3v1 tag, otherwise a table with these string fields:
 * * `title`
 * * `artist`
 * * `album`
 * * `comment`
 * * `year`
 * * `tag`
 * * `genre`
*/
table audioꓸid3v1(audio audio) {}
/**
 * Gets the ID3v2 tag of an .MP3 file
 *
 * Method form: `snd:id3v2()`
 * @param audio The audio (.MP3 only)
 * @return `nil` if there is no ID3v2 tag, otherwise a table with these fields:
 * * **string** `title`
 * * **string** `artist`
 * * **string** `album`
 * * **string** `comment`
 * * **string** `year`
 * * **string** `genre`
 * * **image** `cover`: The album cover (.JPG or .PNG), if there is one
*/
table audioꓸid3v2(audio audio) {}
/**
 * Gets the Vorbis comments (tags) of an .OGG file
 *
 * Method form: `snd:comment()`
 * @param audio The audio (.OGG only)
 * @return `nil` if it isn't an .OGG file, otherwise a table with these string fields:
 * * `title`
 * * `album`
 * * `artist`
 * * `year`
 * * `comment`
 * * `genre`
*/
table audioꓸcomment(audio audio) {}
/**
 * Gets the tags of an .OPUS file
 *
 * Method form: `snd:tags()`
 * @param audio The audio (.OPUS only)
 * @return `nil` if it isn't an .OPUS file, otherwise a table with these fields:
 * * **string** `title`
 * * **string** `album`
 * * **string** `artist`
 * * **string** `date`
 * * **string** `comment`
 * * **string** `genre`
 * * **image** `image`: The front cover, if there is one
*/
table audioꓸtags(audio audio) {}
/** @} */
