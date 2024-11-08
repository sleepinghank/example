use winreg::{RegKey, RegValue};

pub fn test() -> anyhow::Result<()> {
    // 打开 HKEY_CURRENT_USER\Software\YourApp 这个键
    let key_path = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    let hkey = RegKey::predef(winreg::enums::HKEY_CURRENT_USER);
    let mut key = hkey.open_subkey(key_path)?;

    // 设置键值 "ExampleKey" 为字符串 "Some Value"
    key.set_value("InateckOrigin",&"start shell:AppsFolder\\A1232126.InateckOrigin_1tk2hf84048yp!com.inateck.synergy -auto")?;

    // 获取键值 "ExampleKey"
    let value:String = key.get_value("InateckOrigin")?;
    println!("The value of InateckOrigin is: {}", value);

    Ok(())
}