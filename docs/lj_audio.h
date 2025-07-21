/**
 * @file
 * @defgroup audio audio
 * @brief Audio library
 * @{
*/
/**
 * Loads an audio file, supported formats are .MP3, .WAV, .OGG, .FLAC, .OPUS, .AT9 & .AT3
*/
audio audio․load(string path) {}
/**
 * @param loop optional
*/
nil audio․play(audio audio, bool loop) {}
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
 * Detect if audio is playing
*/
boolean audio․playing(audio audio) {}
/**
 * Pause audio
*/
nil audio․pause(audio audio, bool pause) {}
/** @} */