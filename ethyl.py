formats = {
    'pane': '''
            u8 flags_1          // 8
            u8 flags_2          // 9
            u8 alpha_1          // a
            u8 alpha_2          // b
            char name[0x18]     // c
            float x             // 24
            float y             // 28
            float z             // 2c
            float pane_unk1     // 30
            float pane_unk2     // 34
            float angle         // 38
            float xmag          // 3c
            float ymag          // 40
            float width         // 44
            float height        // 48
            etc!
        ''',
    'text': '''
            u16 unk1        // 4c
            u16 unk2        // 4e
            u16 mat_off     // 50
            u16 unk3        // 52
            u8 unk4         // 54
            u8 unk5[3]      // 55
            u32 name_offs   // 58
            u32 unk6        // 5c
            u32 unk7        // 60
            float unk8      // 64
            float unk9      // 68
            float unka      // 6c
            float unkb      // 70
            etc!
        ''',
    'pic': '''
            u32 unk1        // 4c
            u32 unk2        // 50
            u32 unk3        // 54
            u32 unk4        // 58
            u16 mat_off     // 5c
            u8 num_texcoords// 5e, should be < 8
            u8 padding      // 5f
            etc!
        ''',
    'group': '''
            char name[16]
            u16 numsubs
            u16 unk
            etc!
        ''',
        
    'numoffs': '''
            u16 num
            u16 offs
            etc!
        ''',
    'lytheader': '''
            u32 a
            u32 b
            u32 c
        ''',

}