/**
 * @defgroup audio audio
 * @brief Audio library
 * @{
*/
/**
 * Loads an audio file, supported formats are .MP3, .WAV, .OGG, .FLAC, .OPUS, XMP formats (.MOD, .IT, etc.), .AT9 & .AT3
*/
audio audio․load(string path) {}
/**
 * @param loop optional
*/
nil audio․play(audio audio, boolean loop) {}
/**
 * Stops audio
*/
nil audio․stop(audio audio) {}
/**
 * Gets ID3v1 of an .MP3. (only works on .MP3 files!)
 * @return table:
 * * "title"
 * * "artist"
 * * "album"
 * * "comment"
 * * "year"
 * * "tag"
 * * "genre"
*/
table audio․id3v1(audio audio) {}
/**
 * Gets ID3v2 of an .MP3. (only works on .MP3 files!)
*/
table audio․id3v2(audio audio) {}
/**
 * Gets comment of an .OGG. (only works on .OGG files!)
*/
table audio․comment(audio audio) {}
/**
 * Gets tags of an .OPUS. (only works on .OPUS files!)
*/
table audio․tags(audio audio) {}
/**
 * Checks if audio is playing
*/
boolean audio․playing(audio audio) {}
/**
 * Checks if audio is paused
*/
boolean audio․paused(audio audio) {}
/**
 * Pause audio
*/
nil audio․pause(audio audio, boolean pause) {}
/**
 * Audio duration in seconds
*/
number audio․duration(audio audio) {}
/**
 * Elapsed time of audio in seconds
*/
number audio․elapsed(audio audio) {}
/** @} */