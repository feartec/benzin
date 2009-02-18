{
'framesize': 70,
'flags': 1,
'anims': [
    ('RLPA', 'pic', CAT_COORDS, [
        #(COORD_X, [
            #(20.0, 0.0, 1.0),
            #(40.0, 15.0, 1.0)
        #]),
        #(COORD_Y, [
            #(10.0, 0.0, 1.0),
            #(30.0, 15.0, 1.0)
        #]),
        #(COORD_ANGLE, [
            #(0.0, 0.0, 1.0),
            #(70.0, 360.0, 1.0),
        #]),
        (COORD_XMAG, [
            (0.0, 0.5, 0.0),
            (70.0, 0.5, 0.0),
        ]),
        (COORD_YMAG, [
            (0.0,  3.5, 0.0), # Interpolation is weird.  If I specify the same value for the two points and factor = 1, I get a HUGE increase in the middle.
            (70.0, 0.5, 0.0),
        ]),


    ]),
    ('RLVC', 'pic', CAT_COORDS, [
        (0x10, [
            (20.0, 50.0, 1.0),
            (40.0, 200.0, 1.0),
            (60.0, 100.0, 1.0),
        ]),
    ]),


]
}