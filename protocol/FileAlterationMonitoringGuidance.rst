File alteration monitoring
=============================
*Note: Compliance within this section is not required for a client to be compatible with OFP. This section is only meant to provide guidance to the OFP implementor.*

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

