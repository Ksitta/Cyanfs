fn main() -> miette::Result<()> {
    let path = std::path::PathBuf::from("include");
    let mut b = autocxx_build::Builder::new("src/lib.rs", &[&path]).build()?;
    b.flag_if_supported("-std=c++17").compile("binding");
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=include/kv.h");
    println!("cargo:rustc-link-search=/home/nickcao/Projects/filesystem/fs/build/");
    println!("cargo:rustc-link-lib=dylib=kv");
    Ok(())
}
