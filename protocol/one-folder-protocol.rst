one-folder Specification 20140211
=================================
The one-folder protocol (OFP) is a variant of the Bittorrent peer wire protocol.

OFP has files mapped to Bittorrent pieces. OFP uses the Bittorrent protocol to transfer these pieces between peers. The key difference between the vanilla Bittorrent protocol and OFP, is that OFP provides extended Bittorrent PWP messages which allow the mapping, re-mapping, and un-mapping of pieces and files.

Even though OFP is simply just the addition of piece mapping features to the Bittorrent protocol, there are some extra complexities involved. This protocol document describes the OFP protocol, how the piece mapping works, and how complexity is managed.

Table of contents
=================
A client that uses OFP has to solve six key problems. These are elaborated on in the below sections.
1) File alteration monitoring
2) Mapping files to Bittorrent pieces
3) Maintaining file and piece event logs
4) Maintaining log concensus between nodes
5) Transmitting Bittorrent pieces
6) Peer discovery

1. File alteration monitoring
=============================
*Note: Compliance within this section is not required for a client to be compatible with OFP. This section is meant to provide guidance to the OFP client implementor.*

Clients need to monitor the following filesystem events:

Creation
~~~~~~~~
+----------------+----------------------------------------------+
| Parameter name | Comments                                     |
+----------------+----------------------------------------------+
| path           | The path name of the file                    |
+----------------+----------------------------------------------+
| is_dir         | Whether or not the file is a                 |
|                | directory                                    |
+----------------+----------------------------------------------+
| size           | Size of the file in bytes                    |
+----------------+----------------------------------------------+
| mtime          | When the file was last modified              |
+----------------+----------------------------------------------+

Removal
~~~~~~~
+----------------+----------------------------------+
| Parameter name | Comments                         |
+----------------+----------------------------------+
| path           | The full path of the file        |
+----------------+----------------------------------+

Rename
~~~~~~
+----------------+----------------------------------+
| Parameter name | Comments                         |
+----------------+----------------------------------+
| path           | The full path of the file        |
+----------------+----------------------------------+
| new path       | The new path of the file         |
+----------------+----------------------------------+
| mtime          | File's last modified time        |
+----------------+----------------------------------+

Write
~~~~~
+----------------+----------------------------------+
| Parameter name | Comments                         |
+----------------+----------------------------------+
| path           | The full path of the file        |
+----------------+----------------------------------+
| new size       | Size in bytes of file            |
+----------------+----------------------------------+
| mtime          | File's last modified time        |
+----------------+----------------------------------+

2. Mapping files to Bittorrent pieces 
=====================================

 Pieces are referenced by their index, a 32bit unsigned integer.

There are 2^32 possible pieces within one-folder. This is in-line with the Bittorrent protocol.

 Pieces represent data chunks of *up to* 2mb in size.

OFP uses a modified form of the Bittorrent Peer Wire Protocol to allow variable sized pieces. Some files can be held within a single piece, and might not take up all of the piece space. The Piece Log (mentioned in section 3) describes the size each piece.

 Files have a one-to-many relationship with pieces. This relationship is specified by a piece index (unsigned 32bit integer) and piece range (unsigned 32bit integer)
 
This means that files must be mapped to a contiguous range of pieces (the ordering is based off the piece index). For example, readme.txt could have a piece start index of 2 with a piece range of 3, ie. readme.txt is made up of pieces 2, 3, 4, 5. 

 Files are assigned a piece mapping by choosing a random piece index that allows a piece range that supports the entire file's size, without overlaping with any mapped pieces.

The piece range is dependent on the size of the file. For example, you will need at least 5 pieces to represent a 10mb file.

3. Maintaining file and piece event logs
========================================
Three logs are recorded by OFP:

- File log, a listing of file metadata (eg. mapping between files and pieces)
- Piece log, a listing of piece metadata (eg. piece content hashes)  
- Action log, a listing of actions that have been actioned on the File and Piece logs

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
    | is_deleted      | string    | "y" when file has been removed        |
    +-----------------+-----------+---------------------------------------+
    | piece_idx_start | uint32    | The starting piece index of the file  |
    +-----------------+-----------+---------------------------------------+
    | pieces          | uint32    | Number of pieces used by this file    |
    +-----------------+-----------+---------------------------------------+
    | mtime           | uint32    | Last modified time of file meta data  |
    +-----------------+-----------+---------------------------------------+

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
    | mtime          | uint32    | Last modified time of piece meta data |
    +----------------+-----------+---------------------------------------+

Action log
----------
    The action log is a bencoded list of dictionaries. The number of dictionary key/values depends on the "action_type" field. See structure below:
    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | log_id         | uint32    | Unique ID of action                   |
    |                |           |                                       |
    +----------------+-----------+---------------------------------------+
    | action_type    | string    | The type of action                    |
    +----------------+-----------+---------------------------------------+
    | The fields specific to the action are determined by the value of   |
    | the "action_type" field. The action fields are below.              |
    +--------------------------------------------------------------------+

Below is a listing of all the actions types:

map_piece
~~~~~~~~~
A piece is mapped to a file.

    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | file_name      | string    | Path of file                          |
    |                |           |                                       |
    +----------------+-----------+---------------------------------------+
    | offset         | uint32    | Byte offset within file               |
    +----------------+-----------+---------------------------------------+
    | piece_idx      | uint32    | Piece index                           |
    +----------------+-----------+---------------------------------------+

When receiving this message the Client:

    - Updates the "piece_id_start" and/or "piece_id_end" field of the corresponding entry within the File Log.
    - Fail if pieces are not contiguous

unmap_piece
~~~~~~~~~~~
A piece is unmapped. This is done when a file removed, and a piece has a reference count of 0.

    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | file_name      | string    | Path of file                          |
    +----------------+-----------+---------------------------------------+
    | piece_idx      | uint32    | Piece index                           |
    +----------------+-----------+---------------------------------------+

When receiving this message the Client:

    - Updates the "piece_id_start" and/or "piece_id_end" field of the corresponding entry within the File Log.

change_piece
~~~~~~~~~~~~
This occurs when file contents change.

    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | piece_idx      | uint32    | Piece index                           |
    +----------------+-----------+---------------------------------------+
    | new_hash       | string    | New SHA1 hash of piece                |
    +----------------+-----------+---------------------------------------+
    | new_size       | uint32    | New size of piece                     |
    +----------------+-----------+---------------------------------------+

When receiving this message the Client:

    - Updates the "hash" field of the corresponding entry within the Piece Log with "new_hash".
    - Updates the "size" field of the entry from the File Log that points to this piece has its "size" 
    - If "new_hash" is different from "hash" mark piece as "don't have" and send a PWP_DONTHAVE message to one-folder peers

For pieces that haven't changed size the "new_size" value remains the same.

remove_file
~~~~~~~~~~~
This occurs when files are removed.

    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | file_name      | string    | Path of file                          |
    +----------------+-----------+---------------------------------------+

When receiving this message the Client:
    - Updates the corresponding File Log entry's "is_deleted" field to "y".

Deleted files will remain in the file log forever. This is done to ensure that "ghost" copies of deleted files don't reappear unexpectedly.

move_file
~~~~~~~~~
This occurs when files are removed.

    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | path           | string    | Path of file                          |
    +----------------+-----------+---------------------------------------+
    | new_path       | string    | New path of file                      |
    +----------------+-----------+---------------------------------------+

When receiving this message the Client:

    - Updates the corresponding File Log entry's "is_deleted" field to "y".

new_file
~~~~~~~~
This occurs when files are created. 

    +-----------------+-----------+---------------------------------------+
    | Field name      | Data type | Comments                              |
    +-----------------+-----------+---------------------------------------+
    | path            | string    | Path of file                          |
    +-----------------+-----------+---------------------------------------+
    | size            | uint32    | Size (in bytes) of file               |
    +-----------------+-----------+---------------------------------------+
    | piece idx_start | uint32    | The starting piece index of the file  |
    +-----------------+-----------+---------------------------------------+
    | piece idx_end   | uint32    | The ending piece index of the file    |
    +-----------------+-----------+---------------------------------------+
    | mtime           | uint32    | Last modified time of file meta data  |
    +-----------------+-----------+---------------------------------------+

When receiving this message the Client:

    - Inserts a new entry into the File Log using the provided values.

4. Maintaining log concensus (WIP)
==================================
*Note: This section is under review at the moment*

File and Piece logs are shared using one of the following methods:

1. Action log messages
~~~~~~~~~~~~~~~~~~~~~~
If a file event occurs and a peer is connected with non-empty File and Piece logs, an Action log message is sent to the peer.

2. IBLT (Inverse Bloom Lookup Table)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
When a peer first connects with non-empty File and Piece logs:
1) Perform a SHA1 hash on each file and pieces' contents
2) Populate an IBLT with the file and pieces' hashes
3) Send the IBLT to the peer
4) Peer determines the set difference between the received IBLT and their own IBLT
5) Peer sends a message:
 - Request files/pieces for hashes of difference
 - Send IDs of hashes that are different
6) Merge received different items (ie. files/pieces)
7) Update Merkle tree

3. Merkle tree
~~~~~~~~~~~~~~
When a peer first connects and with non-empty File and Piece logs:
1) Send Merkle tree root node
2) Compare to own Merkle tree
3) Request sub trees left breadth first (Merkle tree is ordered by file's name and by piece's index)
4) Recompute Merkle tree as it is updated

4. Full log
~~~~~~~~~~~
Send the full File and Piece logs to the peer.
This is only used when the peer is new to the Shared Folder.

See section 5 for message format.

5. Transmitting Bittorrent pieces
=================================
All messages are sent using the Bittorrent protocol with some specific OFP extensions.

These extensions are below:

Handshake message
~~~~~~~~~~~~~~~~~
Handshake messages are sent at the beginning of the connection. The Bittorrent handshake is not used.
Handshake messages have the following message format:

    +----------------+-----------+----------------------------------------------+
    | Field name     | Data type | Bits | Comments                              |
    +----------------+-----------+----------------------------------------------+
    | protname_len   | byte      |    8 | Length of protocol name               |
    +----------------+-----------+----------------------------------------------+
    | protname       | string    |  N/A | Name of protocol                      |
    +----------------+-----------+----------------------------------------------+

When receiving this message the Client:

    - If handshake is valid, reply with handshake
    - If handshake is invalid, drop connection

File log message
~~~~~~~~~~~~~~~~
File log messages have the following message format:

    +----------------+-----------+----------------------------------------------+
    | Field name     | Data type | Bits | Comments                              |
    +----------------+-----------+----------------------------------------------+
    | len            | uint32    |   32 | length of payload                     |
    +----------------+-----------+----------------------------------------------+
    | msgtype        | byte      |    8 | message type, always equals 9         |
    +----------------+-----------+----------------------------------------------+
    | filelog        | string    |  N/A | Section 2 described bencoded string   |
    +----------------+-----------+----------------------------------------------+

When receiving this message the Client:

    - If we don't have a file that has the same path, we add the file to our
      database, and create the file in our local directory
    - If a file's mtime is less than ours, we ignore the file and enque the file
      info from our database to be sent to the peer

Piece log message
~~~~~~~~~~~~~~~~~
Piece log messages have the following message format:

    +----------------+-----------+----------------------------------------------+
    | Field name     | Data type | Bits | Comments                              |
    +----------------+-----------+----------------------------------------------+
    | len            | uint32    |   32 | length of payload                     |
    +----------------+-----------+----------------------------------------------+
    | msgtype        | byte      |    8 | message type, always equals 10        |
    +----------------+-----------+----------------------------------------------+
    | piecelog       | string    |  N/A | Section 2 described bencoded string   |
    +----------------+-----------+----------------------------------------------+

When receiving this message, we: 

    - add the piece to our database, if we don't have a piece that has the same index database
    - update our database with this piece's info. If a pieces's mtime is higher
      than ours. See below paragraph for how the replacement works
    - we ignore the piece and enque the piece info from our database to be sent
      to the peer, if a pieces's mtime is less than ours, 

If we replace our piece info with a newer piece info, we:

    - send a DONTHAVE message to all our peers, only if we had a complete
      version of the piece before the update. The updated piece index is the
      argument for the message

Don't have Message
~~~~~~~~~~~~~~~~~~
As time goes on, an Action Log entry message might result in a piece not being available on the node anymore.

    A DONTHAVE message is sent to it's peers when the OFP client understands that it doesn't have the up-to-date version of that piece anymore.

    +----------------+-----------+----------------------------------------------+
    | Field name     | Data type | Bits | Comments                              |
    +----------------+-----------+----------------------------------------------+
    | len            | byte      |    8 | Size of payload                       |
    +----------------+-----------+----------------------------------------------+
    | id             | uint32    |   32 | PWP message type, always equals 9     |
    +----------------+-----------+----------------------------------------------+
    | piece id       | uint32    |   32 | The piece index                       |
    +----------------+-----------+----------------------------------------------+

6. Peer discovery
=================
TODO

Folder configurations 
=====================
Shared folders are configured with the following options:

+----------------+----------------------------------------------+---------+
| Parameter name | Comments                                     | Default |
+----------------+----------------------------------------------+---------+
| piece_size     | This is the default size of pieces. If a     | 2mb     |
|                | If a file can be contained within a single   |         |
|                | piece that piece size will be used.          |         |
+----------------+----------------------------------------------+---------+

TODO
====
- Undo log
- Shared secrets
- DHT peer discovery
- LAN broadcast peer discovery
- Encrypted transmission

