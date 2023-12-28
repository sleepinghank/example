#[derive(Default, Debug, Clone)]
#[repr(C)]
pub struct Response{
    pub op_code:u8,
    pub op_error:u8,
    pub op_data:u8,
}

impl Into<Vec<u8>> for Response {
    fn into(self) -> Vec<u8> {
        unsafe { any_as_u8_slice(&self) }.to_vec()
    }
}

impl From<Vec<u8>> for Response {
    fn from(data: Vec<u8>) -> Self {
        let len = unsafe{::core::mem::size_of::<Self>()};
        // TODO: 判断数组长度，如果长度不够，后面自动补0
        println!("{}",len);
        unsafe { std::ptr::read(data.as_ptr() as *const _) }
    }
}



pub fn deserialize_response_row(src: Vec<u8>) -> Response {
    unsafe { std::ptr::read(src.as_ptr() as *const _) }
}

unsafe fn any_as_u8_slice<T: Sized>(p: &T) -> &[u8] {
    ::core::slice::from_raw_parts(
        (p as *const T) as *const u8,
        ::core::mem::size_of::<T>(),
    )
}

fn main() {
    let data = vec![1,2,3];
    let resp = Response::from(data);
    println!("{:?}",resp);
    let v:Vec<u8> = resp.into();
    println!("{:?}",v);

}
