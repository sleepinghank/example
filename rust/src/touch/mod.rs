#[derive(Default,Debug,Clone)]
#[repr(C)]
pub struct Finger {
    tip:    u8,  // 1 bits used
    confidence:      u8, // 1 bits used
    contact_id: u8,  // 6 bits used
    x:      u16, // 12 bits used
    y:         u16,  // 12 bits used
}

#[derive(Default,Debug,Clone)]
#[repr(C)]
pub struct UnmapFinger{
    tab:[u8; 4],
}

impl Finger {
    pub fn unmap(&self) -> UnmapFinger {
        let mut unmap_finger = UnmapFinger { tab: [0; 4]};
        unmap_finger.tab[0] = (self.tip & 0b0000_0001) | (self.confidence << 1) | (self.contact_id << 2) as u8;
        unmap_finger.tab[1] = (self.x & 0b1111_1111) as u8;
        unmap_finger.tab[2] = ((self.x >> 8) & 0b0000_1111)as u8 | (self.y & 0b0000_1111) as u8 ;
        unmap_finger.tab[3] = (self.y >> 4) as u8;
        unmap_finger
    }
}

impl UnmapFinger {
    pub fn map(&self) -> Finger {
        Finger{
            tip: self.tab[0] & 0b0000_0001,
            confidence: (self.tab[0] & 0b0000_0010) >> 1,
            contact_id: (self.tab[0] & 0b1111_1100) >> 2,
            x: self.tab[1] as u16  + (((self.tab[2] & 0b0000_1111 )as u16) <<8),
            y: (self.tab[2] >> 4) as u16  +  ((self.tab[3] as u16) << 4),
        }
    }
}


#[derive(Default,Debug,Clone)]
#[repr(C)]
pub struct Report{
    fingers: [Finger; 4],
    scan_time:u16,
    button:u8,
    button2:u8,
    count:u8,
}

#[derive(Default,Debug,Clone)]
#[repr(C)]
pub struct UnmapReport{
    tab:[u8; 20],
}
impl UnmapReport {
    pub fn map(&self) -> Report {
        let  finger1 = UnmapFinger { tab: [self.tab[0],self.tab[1],self.tab[2],self.tab[3]] }.map();
        let  finger2 = UnmapFinger { tab: [self.tab[4],self.tab[5],self.tab[6],self.tab[7]] }.map();
        let  finger3 = UnmapFinger { tab: [self.tab[8],self.tab[9],self.tab[10],self.tab[11]] }.map();
        let  finger4 = UnmapFinger { tab: [self.tab[12],self.tab[13],self.tab[14],self.tab[15]] }.map();

        Report{
            fingers: [finger1,finger2,finger3,finger4],
            scan_time: self.tab[16] as u16  +  ((self.tab[17] as u16) << 8),
            button: self.tab[18] & 0b0000_0001,
            button2: (self.tab[18] & 0b0000_0010) >> 1,
            count:self.tab[19]
        }
    }
}

#[derive(Default,Debug,Clone)]
#[repr(C)]
pub struct UnmapReportLc{
    tab:[u8; 19],
}
impl UnmapReportLc {
    pub fn map(&self) -> Report {
        let  finger1 = UnmapFinger { tab: [self.tab[0],self.tab[1],self.tab[2],self.tab[3]] }.map();
        let  finger2 = UnmapFinger { tab: [self.tab[4],self.tab[5],self.tab[6],self.tab[7]] }.map();
        let  finger3 = UnmapFinger { tab: [self.tab[8],self.tab[9],self.tab[10],self.tab[11]] }.map();
        let  finger4 = UnmapFinger { tab: [self.tab[12],self.tab[13],self.tab[14],self.tab[15]] }.map();

        Report{
            fingers: [finger1,finger2,finger3,finger4],
            scan_time: self.tab[16] as u16  +  ((self.tab[17] as u16) << 8),
            button: self.tab[18] & 0b0000_0001,
            button2: (self.tab[18] & 0b0000_0010) >> 1,
            count:0
        }
    }
}

#[derive(Default,Debug,Clone)]
#[repr(C)]
pub struct Report2{
    fingers: [Finger; 4],
    scan_time:u16,
    button:u8,
    button2:u8,
}

#[derive(Default,Debug,Clone)]
#[repr(C)]
pub struct UnmapReport2{
    tab:[u8; 20],
}
impl UnmapReport2 {
    pub fn map(&self) -> Report2 {
        let  finger1 = UnmapFinger { tab: [self.tab[4],self.tab[5],self.tab[6],self.tab[7]] }.map();
        let  finger2 = UnmapFinger { tab: [self.tab[8],self.tab[9],self.tab[10],self.tab[11]] }.map();
        let  finger3 = UnmapFinger { tab: [self.tab[12],self.tab[13],self.tab[14],self.tab[15]] }.map();
        let  finger4 = UnmapFinger { tab: [self.tab[16],self.tab[17],self.tab[18],self.tab[19]] }.map();

        Report2{
            scan_time: self.tab[0] as u16  +  ((self.tab[1] as u16) << 8),
            button: self.tab[2] & 0b0000_0001,
            button2: (self.tab[2] & 0b0000_0010) >> 1,
            fingers: [finger1,finger2,finger3,finger4],
        }
    }
}


use std::fs::File;
use std::io::{self, BufRead, BufWriter, Write};
use std::path::Path;


use hex::FromHex;
use anyhow::Result;

fn read_lines<P>(filename: P) -> io::Result<io::Lines<io::BufReader<File>>>
    where P: AsRef<Path>,
{
    let file = File::open(filename)?;
    Ok(io::BufReader::new(file).lines())
}

pub fn touch() -> Result<()>{
    let file_name = "KB09117_error_240305_1141.txt".to_string();
    let lines = read_lines(format!("./src/touch/data/{}",file_name))?;
    let file = File::create(format!("./src/touch/result/{}",file_name))?;
    let mut file = BufWriter::new(file);
    // 使用迭代器，返回一个（可选）字符串
    for line in lines {
        if let Ok(s) = line {
            // if s.len() < 42{
            //     continue;
            // }
            // let s = &s[2..42];
            let decoded = <[u8; 19]>::from_hex(s).expect("Decoding failed");
            let report = UnmapReportLc { tab: decoded }.map();
            // let decoded = <[u8; 20]>::from_hex(s).expect("Decoding failed");
            // let report = UnmapReport2 { tab: decoded }.map();

            println!("button:{},button2:{},scan_time:{},count:{}", report.button,report.button2,report.scan_time,report.count);
            // file.write(format!("button:{},button2:{},scan_time:{}\r\n", report.button,report.button2,report.scan_time).as_bytes()).unwrap();
            // if report.fingers[0].confidence == 0 || report.fingers[1].confidence == 0{
            //     continue;
            // }
            // let x0 = report.fingers[0].x as i32;
            // let x1 = report.fingers[1].x as i32;
            // let dist = x0.wrapping_sub(x1).abs();
            // file.write(format!("{} {}\r\n", dist,report.fingers[0].y).as_bytes()).unwrap();
            // file.write(format!("{} {}\r\n", dist,report.fingers[1].y).as_bytes()).unwrap();
            for finger in report.fingers.iter() {
                // if finger.confidence == 0  {
                //     continue;
                // }
                // let dis = if report.count <= 1 {((finger.x as f32) * 1.6).round () } else { (finger.x as f32)};
                // if ((60.0 - dis / 20.0) > finger.y as f32) && dis > 800.0 && dis < 1200.0{
                //     continue;
                // }
                println!("contact_id:{},tip:{},confidence:{},x:{},y:{}", finger.contact_id,finger.tip,finger.confidence,finger.x,finger.y);
                // file.write(format!("contact_id:{},tip:{},confidence:{},x:{},y:{}\r\n", finger.contact_id,finger.tip,finger.confidence,finger.x,finger.y).as_bytes()).unwrap();
                // file.write(format!("{},{}\r\n",dis,finger.y).as_bytes()).unwrap();
            }
            println!("----------------------------");
            // file.write(format!("----------------------------\r\n").as_bytes()).unwrap();
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    // 注意这个惯用法：在 tests 模块中，从外部作用域导入所有名字。
    use super::*;

    #[test]
    fn test_touch() {
        match touch() {
            Ok(_) => assert!(true),
            Err(e) => {
                println!("{:?}",e);
                assert!(false);
            },
        }
    }


}