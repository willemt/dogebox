Dogebox protocol specification
==============================

Dogebox protocol (DBP) is a variant of the Bittorrent Peer Wire Protocol (PWP).

DBP maps files to Bittorrent pieces, and uses the Bittorrent protocol to
transfer these pieces between peers. The key difference between the vanilla
Bittorrent protocol and DBP, is that DBP provides extended Bittorrent PWP
messages which allow the mapping, re-mapping, and un-mapping of pieces and
files.

Even though DBP is simply just the addition of piece mapping features to the
Bittorrent protocol, there are some extra complexities involved, such as:

- Concensus of the piece log and file log between peers 

Table of contents
=================

A client that implements DBP has to solve six key problems. These are
elaborated on in the below sections.

1) Mapping files to Bittorrent pieces
2) Maintaining file and piece logs
3) Maintaining log concensus between peers
4) Transmitting Bittorrent pieces
5) Peer discovery
6) File alteration monitoring

1. Mapping files to Bittorrent pieces 
=====================================

Pieces are referenced by their index, a 32bit unsigned integer.

 There are 2^32 possible pieces within dogebox. This is in-line with the Bittorrent protocol.

Pieces represent data chunks of *up to* 2mb in size.

 DBP uses a modified form of the Bittorrent Peer Wire Protocol to allow variable
 sized pieces. Some files can be held within a single piece, and might not take
 up all of the piece space. The Piece Log (mentioned in section 2) describes the
 size of each piece.

Files have a one-to-many relationship with pieces. This relationship is
specified by a piece index (unsigned 32bit integer) and a number of pieces
(unsigned 32bit integer). This pair of integers is known as a piece range.
 
 This means that files must be mapped to a contiguous range of pieces (the
 ordering is based off the piece index). For example, "readme.txt" could have a
 piece index of 2 with a piece range length of 4, ie. readme.txt is made up of
 pieces 2, 3, 4, 5.

Files are assigned a piece mapping by choosing a random piece index that allows
a piece range that supports the entire file's size. The file can't have pieces
that overlap with any already mapped pieces.

 The piece range is dependent on the size of the file. For example, you will need
 at least 5 pieces to represent a 10mb file.

2. Maintaining file and piece event logs
========================================
Two logs are recorded by DBP:

- File log, a listing of file metadata (eg. mapping between files and pieces)

- Piece log, a listing of piece metadata (eg. piece content hashes)  

File Log
--------

The file log is a bencoded list of dictionaries with the following key/values:

+-----------------+-----------+---------------------------------------+
| Field name      | Data type | Comments                              |
+-----------------+-----------+---------------------------------------+
| path            | string    | Path of file                          |
+-----------------+-----------+---------------------------------------+
| size            | uint32    | Size of file in bytes                 |
+-----------------+-----------+---------------------------------------+
| piece_idx       | uint32    | Starting piece index of the file      |
+-----------------+-----------+---------------------------------------+
| mtime           | uint32    | Last modified time of file meta data  |
+-----------------+-----------+---------------------------------------+
| utime           | uint32    | The time at which the client detected |
|                 |           | the modification                      |
+-----------------+-----------+---------------------------------------+
| is_deleted      | string    | "y" when file has been removed;       |
|                 |           | "n" otherwise                         |
+-----------------+-----------+---------------------------------------+

Together, "piece_idx" and "size" make up the file's piece range.

Piece Log
---------

The piece log is a bencoded list of dictionaries with the following key/values:

+----------------+-----------+---------------------------------------+
| Field name     | Data type | Comments                              |
+----------------+-----------+---------------------------------------+
| idx            | uint32    | Index of piece                        |
+----------------+-----------+---------------------------------------+
| size           | uint32    | Size of piece in bytes                |
+----------------+-----------+---------------------------------------+
| hash           | string    | SHA1 hashsum of piece contents        |
+----------------+-----------+---------------------------------------+
| mtime          | uint32    | Last modified time of piece metadata  |
+----------------+-----------+---------------------------------------+

3. Maintaining log concensus (WIP)
==================================
Note: This section is under review at the moment

File and Piece logs are shared using one of the following methods:
- IBLT reconcilation & Merkle tree reconciliation
- Merkle tree reconcilation
- Piece & File log transmission

Note: IBLT reconciliation has a risk of false positives, therefore a 2nd round
involving a Merkle tree is required.

IBLT (Inverse Bloom Lookup Table)
---------------------------------
When a peer first connects with non-empty File and Piece logs:

1) Perform a SHA1 hash on each file and pieces' contents

2) Populate an IBLT with the file and pieces' hashes

3) Send the IBLT to the peer

4) Peer determines set difference between received IBLT and their own IBLT

5) Peer sends a message:

 - Request files/pieces for hashes of difference

 - Send IDs of hashes that are different

6) Merge received different items (ie. files/pieces)

7) Update Merkle tree

Merkle tree
-----------
When a peer first connects and with non-empty File and Piece logs:

1) Send Merkle tree root node

2) Compare to own Merkle tree

3) Request sub trees left breadth first (Merkle tree is ordered by file's name and by piece's index)

4) Recompute Merkle tree as it is updated

Piece & File Log Transmission
-----------------------------
Send the full File and Piece logs to the peer.
This is only used when the peer is new to the Shared Folder.

See section 4 for message format.

4. Transmitting Bittorrent pieces
=================================
All messages are sent using the Bittorrent protocol with some specific DBP
extensions.

These extensions are below:

Handshake message
-----------------

Handshake messages are sent at the beginning of the connection.

This message has this structure:

+----------------+-----------+------+---------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+------+---------------------------------------+
| protname_len   | byte      |    8 | Length of protocol name               |
+----------------+-----------+------+---------------------------------------+
| protname       | string    |  N/A | Name of protocol                      |
+----------------+-----------+------+---------------------------------------+
| highest_piece  | uint32    |   32 | The highest piece index that the      |
|                |           |      | client is aware of                    |
+----------------+-----------+------+---------------------------------------+

When receiving this message: 

- If handshake is valid, reply with handshake, and send our piece and
  file log (HS01) 

- If handshake is invalid, drop the connection.


Invalid hanshakes
*****************

Handshakes are treated as invalid when:

- The name length is 0; and/or (HS02) 

- The protocol name is unexpected (HS03)

Highest_piece
*************
This is required within the handshake so that clients are able to construct a
Merkle hash. For a Merkle hash it is necessary that we know how many pieces
there could be.

File log message
----------------

This message has this structure:

+----------------+-----------+------+---------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+------+---------------------------------------+
| len            | uint32    |   32 | length of payload                     |
+----------------+-----------+------+---------------------------------------+
| msgtype        | byte      |    8 | message type, always equals 9         |
+----------------+-----------+------+---------------------------------------+
| filelog        | string    |  N/A | Section 1 described bencoded string   |
+----------------+-----------+------+---------------------------------------+

When receiving this message we process each file dictionary within the bencoded
string, and: 

- If we don't have a file that has the same path, we add the file to
  our database and create the file in our local directory (FL01)

- If the file's mtime is less than ours:

 - We ignore the file and enqueue the file info from our database to be
   sent to the peer. After we've processed the whole file log we send a
   subset of our file log (see below). (FL04)

- If the file's mtime is higher than ours:

 - If we don't have pieces that match the piece range, we add the piece
   range to our database (FL02)

 - When our piece range is different from the file log:
   
  - If the piece range conflicts with our file(s)' piece ranges, we re-map our
    conflicting file(s)' piece ranges and enque the re-mapped file(s) to be
    sent in the file log subset mentioned below (FL06). We then add the new piece
    range to our database (FL03)

 - If the file has a "is_deleted" flag set to "y", we delete the file and
   set our "is_deleted" flag to "y" (FL05) 

File Log subset
******************
This subset consists of files:

- Belonging to us which have a higher mtime than the peer

- That the peer doesn't have

Piece log message
-----------------

This message has this structure:

+----------------+-----------+------+---------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+------+---------------------------------------+
| len            | uint32    |   32 | length of payload                     |
+----------------+-----------+------+---------------------------------------+
| msgtype        | byte      |    8 | message type, always equals 10        |
+----------------+-----------+------+---------------------------------------+
| piecelog       | string    |  N/A | Section 1 described bencoded string   |
+----------------+-----------+------+---------------------------------------+

When receiving this message: 

- If we don't have a piece that has the same index in our database, we
  disconnect (PL01). *(This is because the file log creates the pieces we require.  If
  the Piece Log indicates we need to add pieces, this is most likely a processing error)* 

- We update our database with this piece's info. If a pieces's mtime is
  higher than ours. (PL02) See below paragraph for how the replacement works

- We ignore the piece and enque the piece info from our database to be
  sent to the peer, if a pieces's mtime is less than ours (PL03) 

When we replace our piece info with a newer piece info:

- If we had a complete version of the piece before the update, send a
  DONTHAVE message to all our peers. (PL04) The updated piece index is the argument
  for the message *(We do this to prevent peers from assuming we have the most
  recent piece data)*

Piece Log subset
******************
This subset consists of pieces:

- Belonging to us which have a higher mtime than the peer

- That the peer doesn't have

Don't have Message
------------------

As time goes on, an Action Log entry message might result in a piece not being
available on the node anymore. A DONTHAVE message is sent to it's peers when
the DBP client understands that it doesn't have the up-to-date version of that
piece anymore.

This message has this structure:

+----------------+-----------+------+---------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+------+---------------------------------------+
| len            | byte      |    8 | Size of payload                       |
+----------------+-----------+------+---------------------------------------+
| id             | uint32    |   32 | PWP message type, always equals 9     |
+----------------+-----------+------+---------------------------------------+
| piece id       | uint32    |   32 | The piece index                       |
+----------------+-----------+------+---------------------------------------+

5. Peer discovery (WIP)
=======================


6. File alteration monitoring
=============================
Please see FileAlterationMonitoringGuidance.rst for details.

Todo
====
- Add utime (ie. updated time) to File Log
- Undo log
- Shared secrets
- DHT peer discovery
- LAN broadcast peer discovery
- Encrypted transmission
