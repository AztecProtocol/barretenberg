import ts from 'typescript';
import fs from 'fs';
import { mapDeserializer, mapType } from './mappings.js';
import { toCamelCase } from './to_camel_case.js';

interface Arg {
  name: string;
  type: string;
}

interface FunctionDeclaration {
  functionName: string;
  inArgs: Arg[];
  outArgs: Arg[];
}

function generateCode(filename: string) {
  const fileContent = fs.readFileSync(filename, 'utf-8');
  const functionDeclarations: FunctionDeclaration[] = JSON.parse(fileContent);

  const callWasmExportImports = ts.factory.createImportDeclaration(
    undefined,
    ts.factory.createImportClause(
      false,
      undefined,
      ts.factory.createNamedImports([
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('callWasmExport')),
      ]),
    ),
    ts.factory.createStringLiteral('../call_wasm_export/index.js'),
  );

  ts.addSyntheticLeadingComment(
    callWasmExportImports,
    ts.SyntaxKind.SingleLineCommentTrivia,
    ' WARNING: FILE CODE GENERATED BY BINDGEN UTILITY. DO NOT EDIT!',
    true,
  );

  ts.addSyntheticLeadingComment(
    callWasmExportImports,
    ts.SyntaxKind.MultiLineCommentTrivia,
    ' eslint-disable @typescript-eslint/no-unused-vars ',
    true,
  );

  const serializeImports = ts.factory.createImportDeclaration(
    undefined,
    ts.factory.createImportClause(
      false,
      undefined,
      ts.factory.createNamedImports([
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('BufferDeserializer')),
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('NumberDeserializer')),
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('VectorDeserializer')),
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('BoolDeserializer')),
      ]),
    ),
    ts.factory.createStringLiteral('../serialize/index.js'),
  );

  const typeImport = ts.factory.createImportDeclaration(
    undefined,
    ts.factory.createImportClause(
      false,
      undefined,
      ts.factory.createNamedImports([
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('Fr')),
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('Fq')),
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('Point')),
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('Buffer32')),
        ts.factory.createImportSpecifier(false, undefined, ts.factory.createIdentifier('Buffer128')),
      ]),
    ),
    ts.factory.createStringLiteral('../types/index.js'),
  );

  const statements: ts.Statement[] = [callWasmExportImports, serializeImports, typeImport];

  for (const { functionName, inArgs, outArgs } of functionDeclarations) {
    // const wasmParam = ts.factory.createParameterDeclaration(
    //   undefined,
    //   undefined,
    //   'wasm',
    //   undefined,
    //   ts.factory.createTypeReferenceNode(ts.factory.createIdentifier('WasmModule'), undefined),
    //   undefined
    // );

    const parameters = [
      /*wasmParam,*/ ...inArgs.map(({ name, type }) =>
        ts.factory.createParameterDeclaration(
          /* modifiers */ undefined,
          /* dot dot dot token */ undefined,
          /* name */ toCamelCase(name),
          /* question token */ undefined,
          /* type */ ts.factory.createTypeReferenceNode(mapType(type)),
          /* initializer */ undefined,
        ),
      ),
    ];

    const inArgsVar = ts.factory.createVariableStatement(
      /* modifiers */ undefined,
      ts.factory.createVariableDeclarationList(
        [
          ts.factory.createVariableDeclaration(
            /* name */ ts.factory.createIdentifier('inArgs'),
            /* exclamation token */ undefined,
            /* type */ inArgs.length === 0
              ? ts.factory.createTypeReferenceNode('Array', [
                  ts.factory.createKeywordTypeNode(ts.SyntaxKind.AnyKeyword),
                ])
              : undefined,
            /* expression */ ts.factory.createArrayLiteralExpression(
              inArgs.map(arg => ts.factory.createIdentifier(toCamelCase(arg.name))),
            ),
          ),
        ],
        /* node flags */ ts.NodeFlags.Const,
      ),
    );

    const outTypesVar = ts.factory.createVariableStatement(
      /* modifiers */ undefined,
      ts.factory.createVariableDeclarationList(
        [
          ts.factory.createVariableDeclaration(
            /* name */ ts.factory.createIdentifier('outTypes'),
            /* exclamation token */ undefined,
            /* type */ outArgs.length === 0
              ? ts.factory.createTypeReferenceNode('Array', [
                  ts.factory.createKeywordTypeNode(ts.SyntaxKind.AnyKeyword),
                ])
              : undefined,
            /* expression */ ts.factory.createArrayLiteralExpression(
              outArgs.map(arg => ts.factory.createIdentifier(mapDeserializer(arg.type))),
            ),
          ),
        ],
        /* node flags */ ts.NodeFlags.Const,
      ),
    );

    const wasmCall = ts.factory.createVariableStatement(
      /* modifiers */ undefined,
      /* variable declaration list */ ts.factory.createVariableDeclarationList(
        /* variable declaration array */ [
          ts.factory.createVariableDeclaration(
            /* name */ ts.factory.createIdentifier('result'),
            /* exclamation token */ undefined,
            /* type */ undefined,
            /* initializer */ ts.factory.createCallExpression(
              /* expression */ ts.factory.createIdentifier('callWasmExport'),
              /* type arguments */ undefined,
              /* arguments */ [
                /* function name */ ts.factory.createStringLiteral(functionName),
                /* input arguments */ ts.factory.createIdentifier('inArgs'),
                /* output types */ ts.factory.createIdentifier('outTypes'),
              ],
            ),
          ),
        ],
        /* flags */ ts.NodeFlags.Const,
      ),
    );

    const returnStmt = getReturnStmt(outArgs.length);

    const functionBody = ts.factory.createBlock([inArgsVar, outTypesVar, wasmCall, returnStmt], true);

    const returnType =
      outArgs.length === 0
        ? undefined
        : ts.factory.createTupleTypeNode(outArgs.map(a => ts.factory.createTypeReferenceNode(mapType(a.type))));

    const functionDecl = ts.factory.createFunctionDeclaration(
      /* modifiers */ [ts.factory.createToken(ts.SyntaxKind.ExportKeyword)],
      /* asterisk token */ undefined,
      /* name */ toCamelCase(functionName),
      /* type parameters */ undefined,
      /* parameters */ parameters,
      /* return type */ returnType && outArgs.length === 1 ? returnType.elements.at(0) : returnType,
      /* body */ functionBody,
    );

    statements.push(functionDecl);
  }

  const sourceFile = ts.createSourceFile(
    /* file name */ 'generated.ts',
    /* source code */ '',
    /* script target */ ts.ScriptTarget.Latest,
    /* setParentNodes */ false,
    /* script kind */ ts.ScriptKind.TS,
  );

  const printer = ts.createPrinter();
  const output = printer.printList(ts.ListFormat.MultiLine, ts.factory.createNodeArray(statements), sourceFile);

  console.log(output);
}

function getReturnStmt(numOutArgs: number) {
  switch (numOutArgs) {
    case 0:
      return ts.factory.createReturnStatement();
    case 1:
      return ts.factory.createReturnStatement(
        ts.factory.createElementAccessExpression(
          ts.factory.createIdentifier('result'),
          ts.factory.createNumericLiteral('0'),
        ),
      );
    default:
      return ts.factory.createReturnStatement(
        ts.factory.createAsExpression(
          ts.factory.createIdentifier('result'),
          ts.factory.createKeywordTypeNode(ts.SyntaxKind.AnyKeyword),
        ),
      );
  }
}

generateCode(process.argv[2] || '../exports.json');