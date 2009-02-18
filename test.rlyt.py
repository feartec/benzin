[
    ('lyt1', {
        'a': 1,
        'width': 608.0,
        'height': 456.0
    }),
    ('txl1', {
        'textures': [
            {'unk': 0, 'name': 'smiley.tpl'},
        ]
    }),
    ('fnl1', {
        'files': [
            {'unk': 0, 'name': 'wbf1.brfna'},
            {'unk': 0, 'name': 'wbf2.brfna'},
        ]
    }),
    ('mat1', {
        'materials': [
        {
            'name': 'textmat',
            'tev_color': [0x0, 0x0, 0x0, 0x0], # Possibly background color
            'tev_kcolor': [0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, ],
            'unk_color': [0xff, 0xff, 0xff, 0xff], # Possibly the color of the thing itself
            'unk_color_2': [0xff, 0xff, 0xff, 0xff],
            'texref': [ { 'tex': 'smiley.tpl', 'wrap_s': 0, 'wrap_t': 0 } ], # screwed up with wrap on
            'ua2': [ (0.0, 0.0, 0.0, 1.0, 1.0) ],
            'ua3': [ (1, 4, 0x1e, 0) ], # Changing this --> image disappears (not text, though)
            'ua4': None,
            'ua5': None,
            'ua6': None,
            'ua7': [],
            'ua8': [],
            'ua9': [],
            'uaa': [0x77, 0, 0, 0], # was: [0x77, 0, 0, 0]
            'uab': None,
            
            
        }
     
        ]
    }),
    ('pan1', {
        'alpha': 0xff,
        'alpha_2': 0x0,
        'angle': 0,
        'flags_1': 1,
        'flags_2': 4,
        'width': 608,
        'height': 456,
        'name': 'RootPane',
        'x': 0,
        'y': 0,
        'xmag': 1,
        'ymag': 1,
        'z': 0
    }),
    'pas1',
        ('pic1', {
            'alpha': 0xff,
            'alpha_2': 0x0,
            'angle': 0,
            'flags_1': 1, # Black if omitted
            'flags_2': 4, # "X, Y at center?"
            'width': 152,
            'height': 152,
            'name': 'pic',
            'x': 0,
            'y': 0,
            'xmag': 1,
            'ymag': 1,
            'z': 0,
            '~pic.material': 'textmat',
            '~pic.texcoords': [(0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0)],
        }),
    'pae1',
    ('grp1', {
        'name': 'RootGroup',
        'subs': [],
        'unk': 0
    }),
]