#!/usr/bin/env node

/**
 * Minimal script to style a subgraph on a .dot file
 * `arg1` - original dot file;
 * `arg2` - comma-separated list of node identifiers (optional, by default reads from mapping in *temp.txt*);
 */

const fs = require('fs');
const args = process.argv.slice(2);

if(!args?.length>=1){
    console.log("ERR: no source .dot file in the argument");
    return 0;
}

const dotSourceFile = args[0];
const ctxMappingFile = `temp.txt`;

// console.log(`Reading ${dotSourceFile}...`);
let dot = fs.readFileSync(dotSourceFile,'utf-8');

let obj = {};

if(args.length<2 || typeof args[1] === 'undefined'){
    // console.log(`Reading ${ctxMappingFile}...`);
    let str = fs.readFileSync(ctxMappingFile, 'utf-8');    
    str.split('\n').filter(x=>x).forEach((e,i) => {
        let arr = e.split(':').filter(y=>y);
        obj[arr[0]] = (arr[1]).split(' ').filter(y=>y);
    });
}else{
    args[1].split(',').forEach(x=>{obj[x]=[1]});
}

console.log(obj);


const stylePrefix = `fontcolor="grey",color="grey", style="dashed",`;
dot = dot.split('\n').filter(x=>x).map((line,ind)=>{
    let m;
    m = line.match(/(\d+)->(\d+)/) || line.match(/\d+(?=\[)/)
    if(m?.length==1){       // node
        if(!obj[m[0]]?.length){
            return line.replace("label", `${stylePrefix}label`);
        }
    }else if(m?.length==3){ // edge
        // if(typeof obj[m[1]]==='undefined' || obj?.[m[1]]?.indexOf(m[2])==-1){
        if(typeof obj[m[1]]==='undefined' || typeof obj[m[2]]==='undefined'){
            line=line.replace(`color="blue"`, '');
            return line.replace("label", `${stylePrefix}label`);
        }
    }
    return line;
    
}).join('\n')

fs.writeFileSync(`${dotSourceFile.split('.').slice(0, -1).join('.')}__ctx.dot`,dot);
