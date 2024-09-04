use config::{Config, File};
use std::collections::HashMap;
use std::path::Path;
use anyhow::Result;

pub fn get_keymap() -> Result<HashMap<String, String>> {
    let settings = Config::builder()
        .add_source(File::with_name("./keymap.toml"))
        .build()
        .map_err(|err| anyhow::Error::from(err))?;
    Ok(settings
        .try_deserialize::<HashMap<String, String>>()?)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_keymap() {
        let keymap = get_keymap().unwrap();
        println!("{:?}", keymap);
        assert_eq!(keymap.get("a"), Some(&"KB_A".to_string()));
    }
}