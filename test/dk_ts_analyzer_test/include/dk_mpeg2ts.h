#ifndef _DK_MPEG2TS_H_
#define _DK_MPEG2TS_H_

#include <windows.h>
#include <cstdint>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// compat.h
// Kill deprecation warnings
#pragma warning( 4: 4996 )
#include <io.h>

// On Windows, printf supports %lld but only uses 32 bits of the input value,
// which leads to confusing results. Correct representation of 64 bit integers,
// requires the use of %I64d, which is suitable for printing out offset_t
#define OFFSET_T_FORMAT    "%I64d"
#define OFFSET_T_FORMAT_8 "%8I64d"
#define OFFSET_T_FORMAT_08 "%08I64d"

// Whilst we're at it, define the format for a 64 bit integer as such
#define LLD_FORMAT  "%I64d"
#define LLU_FORMAT  "%I64u"
#define LLD_FORMAT_STUMP "I64d"
#define LLU_FORMAT_STUMP "I64u"

// On Windows, "inline" is a C++ only keyword. In C, it is:
#define inline __inline

// Miscellaneous other Windows-related issues...
#define snprintf _snprintf

#define TRUE  1
#define FALSE 0

// The following defaults are common, and it's difficult
// to decide which other header file they might belong in
#define DEFAULT_VIDEO_PID  0x68
#define DEFAULT_AUDIO_PID  0x67
#define DEFAULT_PMT_PID    0x66

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// h222_fns.h
// Include our function definition(s)
// -- this is actually just the function for returning a string
// representing a stream type (according to the following table),
// which *used* to be a macro, defined in this header file.
extern const char * h222_stream_type_str(uint32_t s);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// h222_defns.h
// ------------------------------------------------------------
// H.222.0 Table 2-29: Stream type assignments, as amended by
// H.222.0 (2000) Amendment 3
//
// Value  Description
// =====  ============================
// 00     ITU-T | ISO/IEC Reserved
// 01     ISO/IEC 11172-2 Video
// 02     ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2
//        constrained parameter video stream
// 03     ISO/IEC 11172-3 Audio
// 04     ISO/IEC 13818-3 Audio
// 05     ITU-T Rec. H.222.0 | ISO/IEC 13818-1 private_sections
// 06     ITU-T Rec. H.222.0 | ISO/IEC 13818-1 PES packets containing
//        private data -- traditionally DVB Dolby (AC-3)
// 07     ISO/IEC 13522 MHEG
// 08     ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A DSM CC
// 09     ITU-T Rec. H.222.1
// 0A     ISO/IEC 13818-6 type A
// 0B     ISO/IEC 13818-6 type B
// 0C     ISO/IEC 13818-6 type C
// 0D     ISO/IEC 13818-6 type D
// 0E     ITU-T Rec. H.222.0 | ISO/IEC 13818-1 auxiliary
// 0F     ISO/IEC 13818-7 Audio with ADTS transport syntax
// 10     ISO/IEC 14496-2 Visual
// 11     ISO/IEC 14496-3 Audio with the LATM transport syntax as defined
//        in ISO/IEC 14496-3 / AMD 1
// 12     ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried
//        in PES packets
// 13     ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried
//        in ISO/IEC14496_sections.
// 14     ISO/IEC 13818-6 Synchronized Download Protocol
// 15     Metadata carried in PES packets
// 16     Metadata carried in metadata_sections
// 17     Metadata carried in ISO/IEC 13818-6 Data Carousel
// 18     Metadata carried in ISO/IEC 13818-6 Object Carousel
// 19     Metadata carried in ISO/IEC 13818-6 Synchronized Download Protocol
// 1A     IPMP stream (defined in ISO/IEC 13818-11, MPEG-2 IPMP)
// 1B     AVC video stream as defined in ITU-T Rec. H.264 | ISO/IEC 14496-10
//        Video
// 1C-7E  ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved
// 7F     IPMP stream
// 80-FF  User Private
// 81	  Traditionally ATSC Dolby (AC-3)
#define MPEG1_VIDEO_STREAM_TYPE			0x01
#define MPEG2_VIDEO_STREAM_TYPE			0x02  // H.262
#define AVC_VIDEO_STREAM_TYPE			0x1B  // MPEG-4 part10 - H.264
#define AVS_VIDEO_STREAM_TYPE           0x42  // AVS -- Chinese standard
#define DVB_DOLBY_AUDIO_STREAM_TYPE		0x06  // [1]
#define ATSC_DOLBY_AUDIO_STREAM_TYPE	0x81  // [1]
#define MPEG2_AUDIO_STREAM_TYPE			0x04
#define MPEG1_AUDIO_STREAM_TYPE			0x03
#define ADTS_AUDIO_STREAM_TYPE			0x0F  // AAC ADTS
#define MPEG4_PART2_VIDEO_STREAM_TYPE   0x10
#define LATM_AUDIO_STREAM_TYPE          0x11  // How much do we support this?
#define DOLBY_DVB_STREAM_TYPE           0x06  // [1]
#define DOLBY_ATSC_STREAM_TYPE          0x81  // [1]
// [1] In DVB (the European transmission standard) Dolby (AC-3) audio is
//     carried in stream type 0x06, but in ATSC (the USA standard), stream
//     type 0x81 is used. Note that both of these are essentially just saying
//     that the data is a private stream, so technically one needs to set
//     descriptors in the PMT as well to say we really mean Dolby (AC-3)
//     Also, in DVB, other types of stream can be in 0x06.

#define IS_VIDEO_STREAM_TYPE(s)  ((s)==MPEG1_VIDEO_STREAM_TYPE || \
                                  (s)==MPEG2_VIDEO_STREAM_TYPE || \
                                  (s)==AVC_VIDEO_STREAM_TYPE   || \
                                  (s)==AVS_VIDEO_STREAM_TYPE   || \
                                  (s)==MPEG4_PART2_VIDEO_STREAM_TYPE)

// Although I include Dolby in the "standard" audio types, beware that the
// stream type usage is not specified by H.222 itself - it is "convention"
// (albeit a standardised convention) how private streams are used to transmit
// Dolby. There is a case to be made that, at any one time, we should not
// recognise *both* potential Dolby stream types, but just one or the other
// (see [1] above) according to the standard the user is expecting. On the
// other hand, practice seems to be to use the stream types only in the
// expected manner.
#define IS_DOLBY_STREAM_TYPE(s) ((s)==DOLBY_DVB_STREAM_TYPE || \
                                 (s)==DOLBY_ATSC_STREAM_TYPE)

#define IS_AUDIO_STREAM_TYPE(s)  ((s)==MPEG1_AUDIO_STREAM_TYPE || \
                                  (s)==MPEG2_AUDIO_STREAM_TYPE || \
                                  (s)==ADTS_AUDIO_STREAM_TYPE  || \
                                  (s)==LATM_AUDIO_STREAM_TYPE  || \
                                  IS_DOLBY_STREAM_TYPE((s)))

// ------------------------------------------------------------
// Stream ids, as used in PES headers
// H.222.0 Table 2-18: Stream_id assignments, as amended by
// H.222.0 (2000) Amendment 3
//
// Note  Hex   stream_id  stream coding
// ====  ===   =========  =============
// 1     BC    1011 1100  program_stream_map
// 2     BD    1011 1101  private_stream_1
//       BE    1011 1110  padding_stream
// 3     BF    1011 1111  private_stream_2
//       C0-DF 110x xxxx  ISO/IEC 13818-3 or ISO/IEC 11172-3 or
//                        ISO/IEC 13818-7 or ISO/IEC 14496-3 audio stream
//                        number x xxxx 
//       Ex    1110 xxxx  ITU-T Rec. H.262 | ISO/IEC 13818-2, ISO/IEC 11172-2,
//                        ISO/IEC 14496-2 or ITU-T Rec. H.264 | ISO/IEC
//                        14496-10 video stream number xxxx
// 3     F0    1111 0000  ECM_stream
//       F1    1111 0001  EMM_stream
// 5     F2    1111 0010  ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A or
//                        ISO/IEC 13818-6_DSMCC_stream
// 2     F3    1111 0011  ISO/IEC_13522_stream
// 6     F4    1111 0100  ITU-T Rec. H.222.1 type A
// 6     F5    1111 0101  ITU-T Rec. H.222.1 type B
// 6     F6    1111 0110  ITU-T Rec. H.222.1 type C
// 6     F7    1111 0111  ITU-T Rec. H.222.1 type D
// 6     F8    1111 1000  ITU-T Rec. H.222.1 type E
// 7     F9    1111 1001  ancillary_stream
//       FA    1111 1010  ISO/IEC14496-1_SL-packetized_stream
//       FB    1111 1011  ISO/IEC14496-1_FlexMux_stream
//       FC    1111 1100  descriptive data stream
//       FD    1111 1101  reserved data stream 
//       FE    1111 1110  reserved data stream
// 4     FF    1111 1111  program_stream_directory
// 
//   The notation x means that the values '0' or '1' are both permitted and
//   results in the same stream type. The stream number is given by the values
//   taken by the x's.
// 
// NOTES
// 1  PES packets of type program_stream_map have unique syntax specified
//    in 2.5.4.1.
// 2  PES packets of type private_stream_1 and ISO/IEC_13552_stream follow
//    the same PES packet syntax as those for ITU-T Rec. H.262 | ISO/IEC
//    13818-2 video and ISO/IEC 13818-3 audio streams.
// 3  PES packets of type private_stream_2, ECM_stream and EMM_stream
//    are similar to private_stream_1 except no syntax is specified after
//    PES_packet_length field.
// 4  PES packets of type program_stream_directory have a unique syntax
//    specified in 2.5.5.
// 5  PES packets of type DSM-CC_stream have a unique syntax specified
//    in ISO/IEC 13818- 6.
// 6  This stream_id is associated with stream_type 0x09 in Table 2-29.
// 7  This stream_id is only used in PES packets, which carry data from
//    a Program Stream or an ISO/IEC 11172-1 System Stream, in a Transport
//    Stream (refer to 2.4.3.7).

#define PADDING_STREAM_ID        0xBE
#define PRIVATE1_AUDIO_STREAM_ID 0xBD
#define PRIVATE2_AUDIO_STREAM_ID 0xBF
#define DEFAULT_VIDEO_STREAM_ID  0xE0   // i.e., stream 0
#define DEFAULT_AUDIO_STREAM_ID  0xC0   // i.e., stream 0

#define IS_AUDIO_STREAM_ID(id)  ((id)==0xBD || ((id) >= 0xC0 && (id) <= 0xDF))
#define IS_VIDEO_STREAM_ID(id)  ((id) >= 0xE0 && (id) <= 0xEF)


#define PADDING_STREAM_ID        0xBE
#define PRIVATE1_AUDIO_STREAM_ID 0xBD
#define PRIVATE2_AUDIO_STREAM_ID 0xBF
#define DEFAULT_VIDEO_STREAM_ID  0xE0   // i.e., stream 0
#define DEFAULT_AUDIO_STREAM_ID  0xC0   // i.e., stream 0

#define IS_AUDIO_STREAM_ID(id)  ((id)==0xBD || ((id) >= 0xC0 && (id) <= 0xDF))
#define IS_VIDEO_STREAM_ID(id)  ((id) >= 0xE0 && (id) <= 0xEF)

// ------------------------------------------------------------
// Timing info (used in reporting on packets). Initialise to all zeroes...
typedef struct _timing_t
{
	uint64_t	first_pcr;
	uint64_t	last_pcr;
	int32_t		first_pcr_packet;
	int32_t		last_pcr_packet;
	int32_t		had_first_pcr;   // FALSE until we've started
} timing_t;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// tswrite_defns.h
// ============================================================
// CIRCULAR BUFFER
// ============================================================

// We default to using a "packet" of 7 transport stream packets because 7*188 =
// 1316, but 8*188 = 1504, and we would like to output as much data as we can
// that is guaranteed to fit into a single ethernet packet, size 1500.
#define DEFAULT_TS_PACKETS_IN_ITEM      7

// For simplicity, we'll have a maximum on that (it allows us to have static
// array sizes in some places). This should be a big enough size to more than
// fill a jumbo packet on a gigabit network.
#define MAX_TS_PACKETS_IN_ITEM          100

// ------------------------------------------------------------
// A circular buffer, usable as a queue
//
// We "waste" one buffer item so that we don't have to maintain a count
// of items in the buffer
//
// To get an understanding of how it works, choose a small BUFFER_SIZE
// (e.g., 11), enable DISPLAY_BUFFER, and select --visual - this will show the
// reading/writing of the circular buffer in action, including the
// "unused item".
//
// The data for the circular buffer
// Each circular buffer item "contains" (up to) N TS packets (where N defaults
// to 7, and is specified as `item_size` in the circular buffer header), and a
// time (in microseconds) when we would like it to be output (relative to the
// time for the first packet "sent").
//
// Said data is stored at the address indicated by the circular buffer
// "header", as `item_data`.
typedef struct _circular_buffer_item_t
{
	uint32_t	time;              // when we would like this data output
	int32_t     discontinuity;     // TRUE if our timeline has "broken"
	int32_t     length;            // number of bytes of data in the array
} circular_buffer_item_t;
#define SIZEOF_CIRCULAR_BUFFER_ITEM sizeof(circular_buffer_item_t)

// ------------------------------------------------------------
// The header for the circular buffer
//
// Note that `start` is only ever written to by the child process, and this is
// the only thing that the child process ever changes in the circular buffer.
//
// `maxnowait` is the maximum number of packets to send to the target host
// without forcing an intermediate wait - required to stop us "swamping" the
// target with too much data, and overrunning its buffers.
typedef struct _circular_buffer_t
{
	int32_t		start;      // start of data "pointer"
	int32_t		end;        // end of data "pointer" (you guessed)
	int32_t		size;       // the actual length of the `item` array
	int32_t		TS_in_item; // max number of TS packets in a circular buffer item
	int32_t		item_size;  // and thus the size of said item's data array
	int32_t		maxnowait;  // max number consecutive packets to send with no wait
	int32_t		waitfor;    // the number of microseconds to wait thereafter
	// The location of the packet data for the circular buffer items
	uint8_t	* item_data;
	// The "header" data for each circular buffer item
	circular_buffer_item_t item[];
} circular_buffer_t;
// Note that size doesn't include the final `item`
#define SIZEOF_CIRCULAR_BUFFER sizeof(circular_buffer_t)
#define DEFAULT_CIRCULAR_BUFFER_SIZE  1024              // used to be 100

// ============================================================
// BUFFERED OUTPUT
// ============================================================

// Information about each TS packet in our circular buffer item
typedef struct _ts_packet_info_t
{
	int32_t		index;
	uint32_t	pid;       // do we need the PIDs?
	int32_t		got_pcr;
	uint64_t	pcr;
} ts_packet_info_t;

#define SIZEOF_TS_PACKET_INFO sizeof(ts_packet_info_t);

// If we're going to support output via our circular buffer in a manner
// similar to that for output to a file or socket, then we need a structure
// to maintain the relevant information. It seems a bit wasteful to burden
// the circular buffer itself with this, particularly as only the writer
// cares about this data, so it needn't be shared.
typedef struct _buffered_ts_output_t
{
	circular_buffer_t *		buffer;
	int32_t					which;   // Which buffer index we're writing to
	int32_t					started; // TRUE if we've started writing therein

	// For each TS packet in the circular buffer, remember its `count`
	// within the input stream, whether it had a PCR, and if so what that
	// PCR was. To make it simpler to access these arrays, also keep a fill
	// index into them (the alternative would be to always re-zero the
	// `got_pcr` values whenever we start a new circular buffer entry,
	// which would be a pain...)
	int32_t					num_packets;  // how many TS packets we've got
	ts_packet_info_t		packet[MAX_TS_PACKETS_IN_ITEM];

	// `rate` is the rate (in bytes per second) we would like to output data at
	uint32_t				rate;

	// `pcr_scale` is a multiplier for PCRs - each PCR found gets its value
	// multiplied by this
	double					pcr_scale;

	// `use_pcrs` indicates if we should use PCRs in the data to drive our
	// timing, rather than use the specified byte rate directly. The `priming`
	// values are only relevant if `use_pcrs` is true.
	int32_t					use_pcrs;

	// 'prime_size' is the amount of space/time to 'prime' the circular buffer
	// output timing mechanism with. This is effectively multiples of the
	// size of a circular buffer item.
	int32_t					prime_size;

	// Percentage "too fast" speedup for our priming rate
	int32_t					prime_speedup;
} buffered_ts_output_t;
#define SIZEOF_BUFFERED_TS_OUTPUT sizeof(buffered_ts_output_t)

// ============================================================
// EXTERNAL DATASTRUCTURES - these are *intended* for external use
// ============================================================

// Our supported target types
// On Unix-type systems, there is little distinction between file and
// socket, but on Windows this becomes more interesting
typedef enum _TS_WRITER_TYPE
{
	TS_W_UNDEFINED,
	TS_W_STDOUT,  // standard output
	TS_W_FILE,    // a file
	TS_W_TCP,     // a socket, over TCP/IP
	TS_W_UDP,     // a socket, over UDP
} TS_WRITER_TYPE;


// ------------------------------------------------------------
// So, *is* it a file or a socket?
typedef union _TS_WRITER_OUTPUT
{
	FILE   *file;
	SOCKET  socket;
} TS_WRITER_OUTPUT;

// ------------------------------------------------------------
// A datastructure to allow us to write to various different types of target
//
// When writing to a file, "how" will be TS_W_STDOUT or TS_W_FILE, and
// "where" will be the appropriate file interface. "writer" is not necessary
// (there's no point in putting a circular buffer and other stuff above
// the file writes), and no child process is needed.
//
// When writing over UDP, "how" will be TS_W_UDP, and "where" will be the
// socket that is being written to. For UDP, timing needs to be managed, and
// thus the circular buffer support is necessary, so "writer" should be
// set to a buffered output context. Since the circular buffer is being
// used, there will also be a child process.
//
// When writing over TCP/IP, "how" will be TS_W_TCP, and "where" will be the
// socket that is being written to. Timing is not an issue, so "writer" will
// not be needed, and nor will there be a child process.  However, it is
// possible that we will want to respond to commands (over the same or another
// socket (or, on Linux/BSD, file descriptor)), so "commander" may be set.
typedef struct _ts_writer_t
{
	TS_WRITER_TYPE				how;    // what type of output we want
	TS_WRITER_OUTPUT			where;  // where it's going to
	buffered_ts_output_t *		writer; // our buffered output interface, if needed
	int32_t						count;  // a count of how many TS packets written

	// Support for the child fork/thread, which actually does the writing when
	// buffered output is enabled.
	HANDLE						child;  // the handle for the child thread (if any)
	int32_t						quiet;  // Should the child be as quiet as possible?

	// Support for "commands" being sent to us via a socket (or, on Linux/BSD,
	// from any other file descriptor). The "normal" way this is used is for
	// our application (tsserve) to act as a server, listening on a socket
	// for an incoming connection, and then both playing data to that
	// connection, and listening for commands from it.
	int32_t						server;         // are we acting as a server?
	SOCKET						command_socket; // where to read commands from/through

	// When the user sends a new command (a different character than is
	// currently in `command`), the underpinnings of tswrite_write() set
	// `command` to that command letter, and `command_changed` to TRUE.
	// Various key functions that write to TS check `command_changed`, and
	// return COMMAND_RETURN_CODE if it is true.
	// Note, however, that it is left up to the top level to *unset*
	// `command_changed` again.
	uint8_t						command;          // A single character "command" for what to do
	int32_t						command_changed;  // Has it changed?
	// Some commands (notably, the "skip" commands) want to be atomic - that
	// is, they should not be interrupted by the user "typing ahead". Since
	// the fast forward and reverse mechanisms (used for skipping as well)
	// call tswrite_command_changed() to tell if there is a new command that
	// should interrup them, we can provide a flag to say "don't do that"...
	int32_t						atomic_command;

	// Should some TS packets be thrown away every <n> packets? This can be
	// useful for debugging other applications
	int32_t						drop_packets;  // 0 to keep all packets, otherwise keep <n> packets
	int32_t						drop_number;   // and then drop this many
} ts_writer_t;
#define SIZEOF_TS_WRITER sizeof(ts_writer_t)

// ------------------------------------------------------------
// Command letters
#define COMMAND_NOT_A_COMMAND '_' // A guaranteed non-command letter

#define COMMAND_QUIT          'q' // quit/exit
#define COMMAND_NORMAL        'n' // normal playing speed
#define COMMAND_PAUSE         'p' // pause until another command
#define COMMAND_FAST          'f' // fast forward
#define COMMAND_FAST_FAST     'F' // faster forward
#define COMMAND_REVERSE       'r' // reverse/rewind
#define COMMAND_FAST_REVERSE  'R' // faster reverse/rewind
#define COMMAND_SKIP_FORWARD       '>'  // aim at 10s
#define COMMAND_SKIP_BACKWARD      '<'  // ditto
#define COMMAND_SKIP_FORWARD_LOTS  ']'  // aim at 100s
#define COMMAND_SKIP_BACKWARD_LOTS '['  // ditto

#define COMMAND_SELECT_FILE_0 '0'
#define COMMAND_SELECT_FILE_1 '1'
#define COMMAND_SELECT_FILE_2 '2'
#define COMMAND_SELECT_FILE_3 '3'
#define COMMAND_SELECT_FILE_4 '4'
#define COMMAND_SELECT_FILE_5 '5'
#define COMMAND_SELECT_FILE_6 '6'
#define COMMAND_SELECT_FILE_7 '7'
#define COMMAND_SELECT_FILE_8 '8'
#define COMMAND_SELECT_FILE_9 '9'

// And a "return code" that means "the command character has changed"
#define COMMAND_RETURN_CODE  -999

// ------------------------------------------------------------
// Context for use in decoding command line - see `tswrite_process_args()`
typedef struct _ts_context_t
{
	// Values used in setting up buffered output
	int32_t		circ_buf_size; // number of buffer entries (+1) for circular buffer
	int32_t		TS_in_item;    // number of TS packets in each circular buffer item
	int32_t		maxnowait;     // max number of packets to send without waiting
	int32_t		waitfor;       // the number of microseconds to wait thereafter
	int32_t		bitrate;       // suggested bit rate  (byterate*8) - both are given
	int32_t		byterate;      // suggested byte rate (bitrate/8)  - for convenience
	int32_t		use_pcrs;      // use PCRs for timing information?
	int32_t		prime_size;    // initial priming size for buffered output
	int32_t		prime_speedup; // percentage of normal speed to prime with
	double		pcr_scale;       // multiplier for PCRs -- see buffered_TS_output
} ts_context_t;

// Arguments processed by tswrite_process_args are set to:
#define TSWRITE_PROCESSED "<processed>"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ts_defns.h
#define TS_PACKET_SIZE 188
#define TS_MAX_PAYLOAD_SIZE (TS_PACKET_SIZE-4)
// The number of TS packets to read ahead
#define TS_READ_AHEAD_COUNT 1024        // aim for multiple of block boundary -- used to be 50
// Thus the number of bytes to read ahead
#define TS_READ_AHEAD_BYTES  TS_READ_AHEAD_COUNT*TS_PACKET_SIZE

typedef struct _dk_ts_reader_t
{
	HANDLE file;
	int64_t position;
	void * handle;

	int32_t (*read_func)(void * p, uint8_t * bytes, size_t * nb);
	int32_t (*seek_func)(void * p, int64_t position);

	uint8_t read_ahead[TS_READ_AHEAD_COUNT*TS_PACKET_SIZE];
	uint8_t * read_ahead_begin;
	uint8_t * read_ahead_end;
} dk_ts_reader_t;

#define SIZEOF_TS_READER sizeof(dk_ts_reader_t)





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ps_defns.h
// ------------------------------------------------------------
// A program stream context, used to read PS and manage a read-ahead cache
#define PS_READ_AHEAD_SIZE  5000  // The number of bytes to read ahead

typedef struct _ps_reader_t
{
	int32_t		input;             // where we're reading from
	int64_t		start;             // the offset at which our data starts
	uint8_t		data[PS_READ_AHEAD_SIZE];
	int64_t		data_posn;         // location of this data in the file
	int32_t		data_len;          // actual number of bytes in the buffer
	uint8_t *	data_end;          // off the end of `data`
	uint8_t *	data_ptr;          // which byte we're interested in (next)
} ps_reader_t;
#define SIZEOF_PS_READER sizeof(ps_reader_t)

// ------------------------------------------------------------
// A program stream pack header (not including the system header packets)
typedef struct _ps_pack_header_t
{
	int32_t		id;            // A number to identify this packet
	uint8_t		data[10];      // The data excluding the leading 00 00 01 BA
	uint64_t	scr;           // Formed from scr_base and scr_ext
	uint64_t	scr_base;
	uint32_t	scr_extn;
	uint32_t	program_mux_rate;
	int32_t		pack_stuffing_length;
} ps_pack_header_t;
#define SIZEOF_PS_PACK_HEADER sizeof(ps_pack_header_t)

// ------------------------------------------------------------
// A program stream packet (specifically one that starts with six bytes
// organised as 00 00 01 <stream id> <packet length>)
typedef struct _ps_packet_t
{
	int32_t		id;            // A number to identify this packet
	uint8_t *	data;          // The data including the leading 00 00 01
	int32_t		data_len;      // Its length
	uint8_t		stream_id;     // Its stream id (i.e., data[4])
	int32_t		packet_length; // The packet length (6 less than data_len)
} ps_packet_t;
#define SIZEOF_PS_PACKET sizeof(ps_packet_t)

// ------------------------------------------------------------
// Number of streams of various types
#define NUMBER_VIDEO_STREAMS    0x0F
#define NUMBER_AUDIO_STREAMS    0x1F
#define NUMBER_AC3_SUBSTREAMS   0x08

// DVD private_stream_1 substream identifiers
// (also used for non-DVD data when we have identified the private data
// appropriately)
#define SUBSTREAM_OTHER         0
#define SUBSTREAM_AC3           1       // AC-3 audio (Dolby 5.1)
#define SUBSTREAM_DTS           2       // DTS  audio
#define SUBSTREAM_LPCM          3       // LPCM audio (CD audio)
#define SUBSTREAM_SUBPICTURES   4       // Sub-pictures
#define SUBSTREAM_ERROR         5       // Error in deciding
#define NUMBER_SUBSTREAM_TYPES  6       // useful for array sizing

#define SUBSTREAM_STR(what)     ((what)==SUBSTREAM_OTHER?"other": \
                                 (what)==SUBSTREAM_AC3  ?"AC3":   \
                                 (what)==SUBSTREAM_DTS  ?"DTS":   \
                                 (what)==SUBSTREAM_LPCM ?"LPCM":  \
                                 (what)==SUBSTREAM_SUBPICTURES?"subpictures": \
                                 "???")

#define SUBSTREAM_IS_AUDIO(what)  ((what)==SUBSTREAM_AC3|| \
                                   (what)==SUBSTREAM_DTS|| \
                                   (what)==SUBSTREAM_LPCM)

#define BSMOD_STR(bsmod,acmod) \
  ((bsmod)==0?"main audio service: complete main (CM)": \
   (bsmod)==1?"main audio service: music & effects (ME)": \
   (bsmod)==2?"associated service: visually impaired (VI)": \
   (bsmod)==3?"associated service: hearing impaired (HI)": \
   (bsmod)==4?"associated service: dialogue (D)": \
   (bsmod)==5?"associated service: commentary (C)": \
   (bsmod)==6?"associated service: emergency (E)": \
   (bsmod)==7 && (acmod)==1?"associated service: voice over (VO)": \
   (bsmod)==7 && (acmod)>=2 && (acmod)<=7?"main audio service: karaoke": \
   "???")

#define ACMOD_STR(acmod) ((acmod)==0?"1+1 Ch1,Ch2": \
                          (acmod)==1?"1/0 C": \
                          (acmod)==2?"2/0 L,R": \
                          (acmod)==3?"3/0 L,C,R": \
                          (acmod)==4?"2/1 L,R,S": \
                          (acmod)==5?"3/1 L,C,R,S": \
                          (acmod)==6?"2/2 L,R,SL,SR": \
                          (acmod)==7?"3/2 L,C,R,SL,SR":"???")




#endif