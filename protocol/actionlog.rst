Action Log Specification 20140219
=================================

Action log, a listing of actions that have been actioned on the File and Piece logs

Action log
----------
    The action log is a bencoded list of dictionaries.
    The number of dictionary key/values depends on the "action_type" field. See structure below:
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

Each peer's action log is independent of other peer's action logs.

Below is a listing of all the action types:

map_piece
~~~~~~~~~
A piece is mapped to a file.

    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | path           | string    | Path of file                          |
    |                |           |                                       |
    +----------------+-----------+---------------------------------------+
    | offset         | uint32    | Byte offset within file               |
    +----------------+-----------+---------------------------------------+
    | piece_idx      | uint32    | Piece index                           |
    +----------------+-----------+---------------------------------------+

When receiving this message we: 

    - update the "piece_idx_start" and/or "pieces" field of the corresponding entry within the File Log.
    - Fail if pieces are not contiguous

unmap_piece
~~~~~~~~~~~
A piece is unmapped. This is done when a file removed, and a piece has a reference count of 0.

    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | path           | string    | Path of file                          |
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

When receiving this action we:

    - update the "hash" field of the corresponding entry within the Piece Log with "new_hash".
    - update the "size" field of the entry from the File Log that points to this piece has its "size" 
    - if "new_hash" is different from "hash" mark piece as "don't have" and send a PWP_DONTHAVE message to one-folder peers

For pieces that haven't changed size the "new_size" value remains the same.

remove_file
~~~~~~~~~~~
This occurs when files are removed.

    +----------------+-----------+---------------------------------------+
    | Field name     | Data type | Comments                              |
    +----------------+-----------+---------------------------------------+
    | path           | string    | Path of file                          |
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
NOTE: new files are not handled by the action log. The client simply sends the new file within a filelog message.

