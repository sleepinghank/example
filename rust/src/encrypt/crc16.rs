fn calc_crc(crc_buf: u8, crc: u16) -> u16 {
    let mut crc = crc ^ &(crc_buf as u16);

    for _ in 0..8 {
        let chk = crc & 1;

        crc >>= 1;
        crc &= 0x7fff;

        if chk == 1 {
            crc ^= 0xa001;
        }

        crc &= 0xffff;
    }

    crc
}

pub fn chk_crc(buf: &[u8]) -> u16 {
    let mut crc = 0xFFFF;

    for &b in buf {
        crc = calc_crc(b, crc);
    }

    let hi = crc % &256;
    let lo = crc / &256;
    crc = (hi << 8) | lo;

    crc
}