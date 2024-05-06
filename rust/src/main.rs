mod port;
mod touch;
mod kalman;
mod encrypt;
mod os;
mod openai;




pub trait Animal {
    fn make_noise(&self) -> String;
}

pub struct Dog;
pub struct Cat;

impl Animal for Dog {
    fn make_noise(&self) -> String {
        "Woof!".to_string()
    }
}

impl Animal for Cat {
    fn make_noise(&self) -> String {
        "Meow!".to_string()
    }
}

#[tokio::main(flavor = "multi_thread", worker_threads = 64)]
async fn main() {
    println!("begin-----------------------------");
    // port::receive_data::test_receive_data();
    // touch::touch().unwrap();
    // let tim = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_secs() as u32;
    // println!("tim: {}", tim);
    // match SystemTime::now().duration_since(UNIX_EPOCH) {
    //     Ok(n) => println!("1970-01-01 00:00:00 UTC was {} seconds ago!", n.as_secs()),
    //     Err(_) => panic!("SystemTime before UNIX EPOCH!"),
    // }

    //     uint8_t a = 51;
    //     uint8_t b = 172;
    //     uint8_t c = a^b;
    //     printf("c:%d\n",c);


    // let dog: Box<dyn Animal> = Box::new(Dog);
    // let cat: Box<dyn Animal> = Box::new(Cat);

    // println!("{}", dog.make_noise());  // 输出："Woof!"
    // println!("{}", cat.make_noise());  // 输出："Meow!"

    // os::hotkaey::test();

    // openai::chat::test("写一篇100的日记".to_string()).await.unwrap();


    // if output.status.success() {
    //     println!("Command executed successfully!");
    //     let stdout = String::from_utf8_lossy(&output.stdout);
    //     println!("Output: {}", stdout);
    // } else {
    //     let stderr = String::from_utf8_lossy(&output.stderr);
    //     eprintln!("Error executing command: {}", stderr);
    // }
    println!("end-----------------------------");
}
