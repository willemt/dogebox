one-folder Specification v0.1
============================

one-folder (OF) makes use of a binary protocol for all synchronisation traffic. A modified version of the Bittorrent protocol is encapsulated within one-folder binary protocol messages.

one-folder has six key problems, which are elaborated on in the below sections.
1) Monitoring filesystem events
2) Mapping files to Bittorrent pieces
3) Maintaining file and piece event logs
4) Maintaining log concensus between nodes
5) Transmitting Bittorrent pieces
6) Peer discovery

1. Monitoring filesystem events
===============================

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
| filename       |                                  |
+----------------+----------------------------------+

Rename
~~~~~~

+----------------+----------------------------------+
| Parameter name | Comments                         |
+----------------+----------------------------------+
| filename       |                                  |
+----------------+----------------------------------+
| new filename   |                                  |
+----------------+----------------------------------+
| mtime          |                                  |
+----------------+----------------------------------+

Write
~~~~~

+----------------+----------------------------------+
| Parameter name | Comments                         |
+----------------+----------------------------------+
| filename       |                                  |
+----------------+----------------------------------+
| new size       |                                  |
+----------------+----------------------------------+
| mtime          |                                  |
+----------------+----------------------------------+

2. Mapping files to Bittorrent pieces 
=====================================
There are 2^32 possible pieces within one-folder. Pieces represent data chunks of *up to* 2mb in size. onefolder uses a modified form of the Bittorrent Peer Wire Protocol to allow variable sized pieces.

Files have a one-to-many relationship with pieces. This relationship is specified by a piece offset and piece range (eg. readme.txt has a piece start IDX of 2 with a piece range of 3, ie. readme.txt is made up of pieces 2, 3, 4, 5). This means that files must be mapped to a contiguous range of pieces (the ordering is based off the piece index).

Files are assigned a piece mapping by choosing a random piece IDX of a piece that isn't yet mapped. The piece range is dependent on the size of the file.

3. Maintaining file and piece event logs
========================================
Three logs are recorded by one-folder:
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
| size            | uint32    | Size if bytes of file                 |
+-----------------+-----------+---------------------------------------+
| is_deleted      | string    | "y" when piece has been removed       |
+-----------------+-----------+---------------------------------------+
| piece_idx_start | uint32    | The starting piece index of the file  |
+-----------------+-----------+---------------------------------------+
| piece_idx_end   | uint32    | The ending piece index of the file    |
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

The receiver:
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

The receiver:
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

The receiver:
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

The receiver:
- Updates the corresponding File Log entry's "is_deleted" field to "y".

Deleted files persist indefinitely in the database. This is done to ensure that "ghost" copies of deleted files don't reappear unexpectedly.

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

The receiver:
- Updates the corresponding File Log entry's "is_deleted" field to "y".

Deleted files persist indefinitely in the database. This is done to ensure that "ghost" copies of deleted files don't reappear unexpectedly.

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

The receiver:
- Inserts a new entry into the File Log using the provided values.

4. Maintaining log concensus (WIP)
==================================
It is compulsory that Action logs be synchronised between nodes from the point that the node.

File and Piece logs are shared using one of the following methods:

1. Action log messages
~~~~~~~~~~~~~~~~~~~~~~
If a file event occurs and a peer is connected with non-empty File and Piece logs, an Action log message is sent to the peer.

2. IBLT (Inverse Bloom Lookup Table)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
When a peer first connects and with non-empty File and Piece logs:
1) Hash files
2) Populate IBLT with hashes. Avoid collisions
3) Send IBLT
4) Figure out set difference
5) One MSG:
 - Request IDs for hashes of difference
 - Send IDs that are different
6) Merge received different items 
7) Update merkle tree

3. Merkle tree
~~~~~~~~~~~~~~
When a peer first connects and with non-empty File and Piece logs:
1) Send root node
2) Compare to own tree
3) Request sub trees left breadth first (merkle tree is ordered by file's name)
4) Recompute merkle tree as it is updated

4. Full log
~~~~~~~~~~~
Send the full File and Piece logs to the peer.
This is only used when the peer is new to the Shared Folder.

See section 5 for message format.

5. Transmitting Bittorrent pieces
=================================
The bittorrent protocol is encapsulated within one-folder messages.

Handshake message
~~~~~~~~~~~~~~~~~
Handshake messages are sent at the beginning of the connection.
Handshake messages have the following message format:

+----------------+-----------+----------------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+----------------------------------------------+
| protname_len   | byte      |    8 | Length of protocol name               |
+----------------+-----------+----------------------------------------------+
| protname       | string    |  N/A | Name of protocol                      |
+----------------+-----------+----------------------------------------------+

The receiver:
- If handshake is valid, reply with handshake
- If handshake is invalid, drop connection

Keep alive message
~~~~~~~~~~~~~~~~~~
Keep alive messages ensure the connection with the peer is kept open.

+----------------+-----------+----------------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+----------------------------------------------+
| of_id          | byte      |    8 | message type, always equals 0         |
+----------------+-----------+----------------------------------------------+

Full log message
~~~~~~~~~~~~~~~~
Full log messages have the following message format:

+----------------+-----------+----------------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+----------------------------------------------+
| of_id          | byte      |    8 | message type, always equals 1         |
+----------------+-----------+----------------------------------------------+
| filelog_len    | uint32    |   32 | Length of file log string             |
+----------------+-----------+----------------------------------------------+
| piecelog_len   | uint32    |   32 | Length of piece log string            |
+----------------+-----------+----------------------------------------------+
| filelog        | string    |  N/A | Bencoded string                       |
+----------------+-----------+----------------------------------------------+
| piecelog       | string    |  N/A | Bencoded string                       |
+----------------+-----------+----------------------------------------------+

Bittorrent message
~~~~~~~~~~~~~~~~~~
one-folder bittorrent messages have the following message format:

+----------------+-----------+----------------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+----------------------------------------------+
| of_id          | byte      |    8 | message type, always equals 2         |
+----------------+-----------+----------------------------------------------+
| payload_len    | uint32    |   32 | Length of the PWP payload in bytes    |
+----------------+-----------+----------------------------------------------+
| payload        | Bittorrent PWP message data                              |
+----------------+----------------------------------------------------------+

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

one-folder Peer Wire Protocol extensions
========================================
If a peer also uses one-folder, the one-folder Peer Wire Protocol extensions are used.

PWP_DONTHAVE Message
--------------------
As time goes on, an Action Log entry message might result in a piece not being available on the node anymore. A PWP_DONTHAVE message is sent when the one-folder client understands that it doesn't have that piece anymore.

+----------------+-----------+----------------------------------------------+
| Field name     | Data type | Bits | Comments                              |
+----------------+-----------+----------------------------------------------+
| len            | byte      |    8 | Size of payload                       |
+----------------+-----------+----------------------------------------------+
| id             | uint32    |   32 | PWP message type, always equals 9     |
+----------------+-----------+----------------------------------------------+
| piece id       | uint32    |   32 | The piece index                       |
+----------------+-----------+----------------------------------------------+

TODO
====
- Undo log
- Shared secrets
- DHT peer discovery
- LAN broadcast peer discovery
- Encrypted piece transmission

Current implementation
======================
The following functions are enough for the filesystem module to provide the necessary functionality:
.. code:: python
    def new_file(file, size hint)
    def enlarge_file(file, new_size)
    def reduce_size(file, new_size)
    def delete_file(file)
    def filepos_getpiece(file, pos)

