
import sys, os

def options(opt):
        opt.load('compiler_c')

contribs = [
('fff', 'http://github.com/willemt/fff'),
('CBag', 'http://github.com/willemt/CBag'),
('CHeap','http://github.com/willemt/CHeap'),
('libuv','http://github.com/joyent/libuv'),
('CAVLTree', 'http://github.com/willemt/CAVLTree'),
('CBitfield', 'http://github.com/willemt/CBitfield'),
('CPSeudoLRU','http://github.com/willemt/CPseudoLRU'),
('CMeanQueue','http://github.com/willemt/CMeanQueue'),
('CBipBuffer', 'http://github.com/willemt/CBipBuffer'),
('YABTorrent', 'http://github.com/willemt/YABTorrent'),
('CConfig-re', 'http://github.com/willemt/CConfig-re'),
('CEventTimer', 'http://github.com/willemt/CEventTimer'),
('CTrackerClient', 'http://github.com/willemt/CTrackerClient'),
('CSparseCounter', 'http://github.com/willemt/CSparseCounter'),
('CLinkedListQueue', 'http://github.com/willemt/CLinkedListQueue'),
('CSimpleBitstream', 'http://github.com/willemt/CSimpleBitstream'),
('PeerWireProtocol', 'http://github.com/willemt/PeerWireProtocol'),
#('CTorrentFileReader', 'http://github.com/willemt/CTorrentFileReader'),
('CSparseFileAllocator', 'http://github.com/willemt/CSparseFileAllocator'),
('CHashMapViaLinkedList','http://github.com/willemt/CHashMapViaLinkedList'),
#('CHeaplessBencodeReader', 'http://github.com/willemt/CHeaplessBencodeReader'),
('CStreamingBencodeReader', 'http://github.com/willemt/CStreamingBencodeReader'),
]

def configure(conf):
    conf.load('compiler_c')
    if sys.platform == 'win32':
        conf.check_cc(lib='ws2_32')
        conf.check_cc(lib='psapi')

    conf.env.STDLIBPATH = ['.']
#    conf.check_cc(lib='uv')

    conf.find_program("git")

    # Get the required contributions via GIT
    for c in contribs:
        if os.path.exists("./"+c[0]):
            conf.env.CONTRIB_PATH = './'
            print "Using git to update %s..." % c[1]
            try:
                conf.cmd_and_log("git pull %s" % c[1], cwd=c[0])
            except Exception as e:
                print e.stdout +' '+ e.stderr
                conf.fatal("Couldn't update ./%s from %s using git" % (c[1],c[0]))
        elif not os.path.exists("../"+c[0]):
            conf.env.CONTRIB_PATH = './'
            print "Using git to clone %s..." % c[1]
            try:
                conf.cmd_and_log("git clone %s %s" % (c[1],c[0],))
            except Exception as e:
                print e.stdout +' '+ e.stderr
                conf.fatal("Couldn't clone %s using git" % c[0])
        else:
            conf.env.CONTRIB_PATH = '../'
            
    print "Configuring libuv (autogen.sh)"
    conf.exec_command("sh autogen.sh", cwd="libuv")
    print "Configuring libuv (configure)"
    conf.exec_command("sh configure", cwd="libuv")
    print "Building libuv.a"
    conf.exec_command("make", cwd="libuv")
    conf.exec_command("mkdir build")
    conf.exec_command("cp libuv/.libs/libuv.a build/")


from waflib.Task import Task

class compiletest(Task):
    def run(self):
        return self.exec_command('sh ../make-tests.sh %s > %s' % (
                            self.inputs[0].abspath(),
                            self.outputs[0].abspath()))

def unit_test(bld, src, ccflag=None):
    # collect tests into one area
    bld(rule='sh ../make-tests.sh ../tests/'+src+' > ${TGT}', target="tests/t_"+src)

    libs = []

    # build the test program
    bld.program(
        source=[
            "tests/"+src,
            "tests/t_"+src,
            'tests/CuTest.c',
            bld.env.CONTRIB_PATH+"CBitfield/bitfield.c",
            bld.env.CONTRIB_PATH+"CSimpleBitstream/bitstream.c",
        ],
        target=src[:-2],
        cflags=[
            '-g',
            '-Werror'
        ],
        use='yabbt',
        lib = libs,
        unit_test='yes',
        includes=[
            "./include",
            bld.env.CONTRIB_PATH+"CBitfield",
            bld.env.CONTRIB_PATH+"CSimpleBitstream",
            bld.env.CONTRIB_PATH+"YABTorrent/include",
        ]
        )

    # run the test
    if sys.platform == 'win32':
        bld(rule='${SRC}',source=src[:-2]+'.exe')
    else:
        bld(rule='export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. && ./${SRC}', source=src[:-2])


def scenario_test(bld, src, ccflag=None):
    bld(rule='cp ../make-tests.sh .')
    bld(rule='cp ../%s .' % src)
    # collect tests into one area
    bld(rule='sh ../make-tests.sh '+src+' > ${TGT}', target="t_"+src)

    libs = []
    bld.program(
        source=[
            src,
            "t_"+src,
            'CuTest.c',
            "networkfuncs_mock.c",
            "mt19937ar.c",
            "mock_torrent.c",
            "mock_client.c",
            bld.env.CONTRIB_PATH+"CBipBuffer/bipbuffer.c"
            ],
        stlibpath = ['libuv','.'],
        target=src[:-2],
        cflags=[
            '-g',
            '-Werror',
            '-Werror=uninitialized',
            '-Werror=return-type'
            ],
        lib = libs,
        unit_test='yes',
        includes=[
            bld.env.CONTRIB_PATH+"CConfig-re",
            bld.env.CONTRIB_PATH+"CBTTrackerClient",
            #bld.env.CONTRIB_PATH+"CHeaplessBencodeReader",
            bld.env.CONTRIB_PATH+"CStreamingBencodeReader",
            #bld.env.CONTRIB_PATH+"CTorrentFileReader",
            bld.env.CONTRIB_PATH+"CHashMapViaLinkedList",
            bld.env.CONTRIB_PATH+"CBipBuffer",
           ], 
        use='yabbt')

    # run the test
    if sys.platform == 'win32':
        bld(rule='${SRC}',source=src[:-2]+'.exe')
    else:
        bld(rule='export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. && ./${SRC}',source=src[:-2])
        #bld(rule='pwd && export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. && ./'+src[:-2])

def build(bld):
    cp = bld.env.CONTRIB_PATH

    # Copy libuv.a to build/
    bld(rule='cp ../libuv/.libs/libuv.a .', always=True)#, target="libuv.a")

    libs = []

    if sys.platform == 'win32':
        platform = '-DWIN32'
    elif sys.platform == 'linux2':
        platform = '-DLINUX'
    else:
        platform = ''

    bld.shlib(
        source= [
            "src/file2piece_mapper.c",
            "src/handshaker.c",
            "src/connection.c",
            "src/connection_filelog.c",
            "src/connection_piecelog.c",
            "src/msghandler.c",
            cp+"YABTorrent/src/bt_download_manager.c",
            cp+"YABTorrent/src/bt_peer_manager.c",
            cp+"YABTorrent/src/bt_piece.c",
            cp+"YABTorrent/src/bt_diskmem.c",
            cp+"YABTorrent/src/bt_piece_db.c",
            cp+"YABTorrent/src/bt_blacklist.c",
            cp+"YABTorrent/src/bt_diskcache.c",
            cp+"YABTorrent/src/bt_filedumper.c",
            cp+"YABTorrent/src/bt_choker_seeder.c",
            cp+"YABTorrent/src/bt_choker_leecher.c",
            cp+"YABTorrent/src/bt_selector_random.c",
            cp+"YABTorrent/src/bt_selector_rarestfirst.c",
            cp+"YABTorrent/src/bt_selector_sequential.c",
            cp+"YABTorrent/src/bt_util.c",
            cp+"YABTorrent/src/bt_sha1.c",
            cp+"YABTorrent/src/bt_string.c",
            #cp+"YABTorrent/readfile.c",
            cp+"YABTorrent/src/sha1.c",
            cp+"CBag/bag.c",
            cp+"CHeap/heap.c",
            cp+"CConfig-re/list.c",
            cp+"CAVLTree/avl_tree.c",
            cp+"CConfig-re/config.c",
            cp+"CBitfield/bitfield.c",
            cp+"CMeanQueue/meanqueue.c",
            cp+"CPSeudoLRU/pseudolru.c",
            cp+"CEventTimer/event_timer.c",
            cp+"CSimpleBitstream/bitstream.c",
            cp+"CSparseCounter/sparse_counter.c",
            #cp+"CHeaplessBencodeReader/bencode.c",
            cp+"CStreamingBencodeReader/bencode.c",
            cp+"PeerWireProtocol/pwp_connection.c",
            cp+"PeerWireProtocol/pwp_util.c",
            cp+"PeerWireProtocol/pwp_bitfield.c",
            cp+"PeerWireProtocol/pwp_msghandler.c",
            cp+"PeerWireProtocol/pwp_handshaker.c",
            cp+"CLinkedListQueue/linked_list_queue.c",
            #cp+"CTorrentFileReader/torrentfile_reader.c",
            cp+"CHashMapViaLinkedList/linked_list_hashmap.c",
            cp+"CSparseFileAllocator/sparsefile_allocator.c",
            ],
        #bt_diskmem.c
        #CCircularBuffer/cbuffer.c
        #use='config',
        target='yabbt',
        lib = libs,
        includes=[
            './include',
            cp+"CBag",
            cp+"CHeap",
            cp+"CAVLTree",
            cp+"CBitfield",
            cp+"CMeanQueue",
            cp+"CPSeudoLRU",
            cp+"CBipBuffer",
            cp+"CConfig-re",
            cp+"YABTorrent/include",
            cp+"CEventTimer",
            cp+"CSparseCounter",
            cp+"CTrackerClient",
            cp+"CLinkedListQueue",
            cp+"PeerWireProtocol",
            cp+"CSimpleBitstream",
            cp+"CSparseFileAllocator",
            cp+"CHashMapViaLinkedList",
            #cp+"CHeaplessBencodeReader",
            cp+"CStreamingBencodeReader",
           ], 
        cflags=[
            '-Werror',
            '-Werror=format',
            '-Werror=int-to-pointer-cast',
            '-g',
            platform,
            '-Werror=return-type',
            '-Werror=uninitialized',
            '-Werror=pointer-to-int-cast',
            '-Wcast-align'],
        )

    unit_test(bld,'test_msghandler.c')
    unit_test(bld,'test_f2p.c')
    unit_test(bld,'test_handshaker.c')
    unit_test(bld,'test_connection.c')

    libs = ['yabbt','uv']
    if sys.platform == 'win32':
        libs += ['ws2_32']
        libs += ['psapi']
        libs += ['Iphlpapi']
    else:
        libs += ['dl']
        libs += ['rt']
        libs += ['pthread']
        # for log() and pow()
        libs += ['m']

    bld.program(
        source=[
            "src/main.c",
            cp+"CBitfield/bitfield.c",
            cp+"fff/fff.c",
            cp+"CBipBuffer/bipbuffer.c",
            cp+"YABTorrent/src/mt19937ar.c",
            cp+"YABTorrent/src/networkfuncs_libuv.c",
            ],
        target='dogebox',
        cflags=[
            '-g',
            '-Werror',
            '-Werror=uninitialized',
            '-Werror=pointer-to-int-cast',
            '-Werror=return-type'
            ],
        stlibpath = ['./libuv','.'],
        lib = libs,
        includes=[
            './include',
            './libuv/include',
            cp+"fff",
            # needed by fff
            cp+"CHeap",
            cp+"CBitfield",
            cp+"CConfig-re",
            cp+"CBipBuffer",
            cp+"CSparseCounter",
            cp+"YABTorrent/include",
            cp+"CSimpleBitstream",
            cp+"PeerWireProtocol",
            cp+"CLinkedListQueue",
            #cp+"CTorrentFileReader",
            cp+"CHashMapViaLinkedList",
            #cp+"CHeaplessBencodeReader",
            cp+"CStreamingBencodeReader",
           ])

