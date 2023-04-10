use std::env;
use std::fs;
use serde::Deserialize;
use inflector::Inflector;

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct Arg {
    name: String,
    #[serde(rename = "type")]
    arg_type: String,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct FunctionDeclaration {
    function_name: String,
    in_args: Vec<Arg>,
    out_args: Vec<Arg>,
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let default_filename = String::from("../exports.json");
    let filename = args.get(1).unwrap_or(&default_filename);
    let file_content = fs::read_to_string(filename).expect("Error reading file");

    let function_declarations: Vec<FunctionDeclaration> = serde_json::from_str(&file_content).expect("Error parsing JSON");

    let mut code = String::new();

    code.push_str("use wasm_bindgen::prelude::*;\n");
    code.push_str("use wasm_bindgen::JsValue;\n");
    code.push_str("use crate::serialize::{BufferDeserializer, NumberDeserializer, VectorDeserializer, BoolDeserializer};\n");
    code.push_str("use crate::types::{Fr, Fq, Point, Buffer32, Buffer128};\n");

    for FunctionDeclaration { function_name, in_args, out_args } in function_declarations {
        let fn_name = function_name.to_camel_case();
        let in_args_str = in_args.iter().map(|arg| format!("{}: {}", arg.name.to_snake_case(), map_type(&arg.arg_type))).collect::<Vec<String>>().join(", ");
        let out_args_tuple = if out_args.len() > 1 { format!("({})", out_args.iter().map(|arg| map_type(&arg.arg_type)).collect::<Vec<String>>().join(", ")) } else { String::from("") };
        let out_args_return_type = if out_args.len() == 1 { map_type(&out_args[0].arg_type) } else { out_args_tuple };

        code.push_str(&format!("#[wasm_bindgen]\npub fn {}({}) -> {} {{\n", fn_name, in_args_str, out_args_return_type));
        code.push_str("    let in_args = js_sys::Array::new();\n");

        for arg in &in_args {
            code.push_str(&format!("    in_args.push({}.into());\n", arg.name.to_snake_case()));
        }

        code.push_str("    let out_types = js_sys::Array::new();\n");

        for arg in &out_args {
            code.push_str(&format!("    out_types.push({}.into());\n", map_deserializer(&arg.arg_type)));
        }

        code.push_str(&format!("    let result = crate::call_wasm_export::call_wasm_export(\"{}\", in_args, out_types).unwrap();\n", function_name));

        if out_args.len() > 0 {
            code.push_str("    return result;\n");
        }

        code.push_str("}\n");
    }

    println!("{}", code);
}

fn map_type(arg_type: &str) -> String {
    match arg_type {
        "fr::in_buf" => "Fr",
        "fr::out_buf" => "Fr",
        "fr::vec_in_buf" => "Fr[]",
        "fr::vec_out_buf" => "Fr[]",
        "fq::in_buf" => "Fq",
        "fq::out_buf" => "Fq",
        "fq::vec_in_buf" => "Fq[]",
        "fq::vec_out_buf" => "Fq[]",
        "const uint8_t *" => "Buffer",
        "uint8_t **" => "Buffer",
        "in_buf32" => "Buffer32",
        "out_buf32" => "Buffer32",
        "uint32_t" => "number",
        "const uint32_t *" => "number",
        "affine_element::in_buf" => "Point",
        "affine_element::out_buf" => "Point",
        "const bool *" => "boolean",
        "bool *" => "boolean",
        "multisig::MultiSigPublicKey::vec_in_buf" => "Buffer128[]",
        "multisig::MultiSigPublicKey::out_buf" => "Buffer128",
        "multisig::RoundOnePublicOutput::vec_in_buf" => "Buffer128[]",
        "multisig::RoundOnePublicOutput::out_buf" => "Buffer128",
        "multisig::RoundOnePrivateOutput::in_buf" => "Buffer128",
        "multisig::RoundOnePrivateOutput::out_buf" => "Buffer128",
        _ => panic!("Unknown type"),
    }.to_string()
}

fn map_deserializer(arg_type: &str) -> String {
    match arg_type {
        "fr::out_buf" => "Fr",
        "fr::vec_out_buf" => "VectorDeserializer(Fr)",
        "fq::out_buf" => "Fq",
        "fq::vec_out_buf" => "VectorDeserializer(Fq)",
        "uint8_t **" => "BufferDeserializer()",
        "out_buf32" => "Buffer32",
        "uint32_t" => "NumberDeserializer()",
        "affine_element::out_buf" => "Point",
        "bool *" => "BoolDeserializer()",
        "multisig::MultiSigPublicKey::out_buf" => "Buffer128",
        "multisig::RoundOnePublicOutput::out_buf" => "Buffer128",
        "multisig::RoundOnePrivateOutput::out_buf" => "Buffer128",
        _ => panic!("Unknown deserializer"),
    }.to_string()
}