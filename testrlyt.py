[
    ('lyt1', {
        'a': 1,
        'width': 608.0,
        'height': 456.0
    }),
    ('txl1', {
        'textures': [
            {'unk': 0, 'name': ''},
            {'unk': 1, 'name': 'foo1'},
            {'unk': 0, 'name': 'foo2'},
            {'unk': 0, 'name': 'foo3'},
        ]
    }),
    ('fnl1', {
        'files': [
            {'unk': 0, 'name': 'your mom'}
        ]
    }),
    ('mat1', {
        'materials': ['textmat']
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
        ('txt1', {
            'alpha': 0xff,
            'alpha_2': 0x0,
            'angle': 0,
            'flags_1': 5,
            'flags_2': 4,
            'height': 38,
            'name': 'textbox',
            'width': 400,
            'x': 0,
            'y': 50,
            'xmag': 1,
            'ymag': 2,
            'z': 0,
            '~text.material': 'textmat',
            '~text.text': 'Some Text',
            '~text.unk3': 0,
            '~text.unk4': 4,
            
            '~text.unk6': 0xffffff,
            '~text.unk7': 0x80ff,
            '~text.font_size_x': 32.0,
            '~text.font_size_y': 38.0,
            '~text.char_space': -1.0,
            '~text.line_space': -1.0,
        }),
        ('pic1', {
            'alpha': 0xff,
            'alpha_2': 0x0,
            'angle': 0,
            'flags_1': 5,
            'flags_2': 4,
            'height': 38,
            'name': 'pic',
            'width': 400,
            'x': 200,
            'y': 50,
            'xmag': 1,
            'ymag': 2,
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
    'grs1',
        ('grp1', {
            'name': 'ENG',
            'subs': ['textbox', 'pic'],
            'unk': 0
        }),
    'gre1',

]