const fs = require('fs');
const path = require('path');
const util = require('util')

let arr = fs.readdirSync('.', {
        encoding:'utf-8',
        withFileTypes:true
    }).filter(entry=>{
        return entry.isDirectory()
    }).flatMap( (curr)=>(
        fs.readdirSync(
                path.join('.', curr.name),'utf-8'
            ).map(
                x=>path.join('.', curr.name, x)
            )
        )
    );

let obj = {};


for(const fname of arr){
    let lines = fs.readFileSync(fname,'utf-8').split('\n').filter(x=>x);
    
    let isMetaG = 0;
    let isMetaV = 0;

    let isG=0;
    let isGV=0;
    let isV=0;

    if(fname.match(/gen.txt$/)){
        isG=1;
    }
    if(fname.match(/gverif.txt$/)){
        isGV=1;
    }
    if(fname.match(/verif.txt$/)){
        isV=1;
    }

    if(isG || isGV || isV){
        lines.forEach(line=>{
            let pair = line.split(':');
            let key = pair[0].replace(/[\s]/g,'');
            let vals = pair[1].replace(/[\[\]]/g,'').split(/[\s]+/).filter(x=>x).map(x=>Number(x))
            if(!obj[key]){
                obj[key]={}
            }         

            if(isG){
                obj[key]["gen"] = vals;
            }else if(isGV){
                obj[key]["gverif"] = vals;
            }else if(isV){
                obj[key]["verif"] = vals;
            }
        })
    }

    if(fname.match(/metag.txt$/)){
        isMetaG=1
    }
    if(fname.match(/metav.txt$/)){
        isMetaV=1;
    }


    if(isMetaG || isMetaV){
        lines = lines.join('\n')
        if(isMetaV){
            lines=lines.replace(/Verification result: OK/g,'').replace(/Verification result: ERR/g,'')
        }

        lines = lines.replace(/([^\[\]\s]*):/g,'"$1":').replace(/\](?=[^\]])/g,'\],').replace(/\]/g,'\}').replace(/\[/g,'\{')
        // console.log((lines));
        // console.log('===============================');
        
        lines = lines.replace(/\{([\d,\s]+)\}/g, "[$1]")
        // console.log((lines));
        // console.log('==============================');
        
        lines = "{"+lines+"}"
        // console.log((lines));
        // console.log('==============================');
        // console.log(JSON.parse(lines));

        let temp = JSON.parse(lines);

        for(const key in temp){
            if(!obj[key]){
                obj[key]={}
            }   
            obj[key][isMetaG ? 'metag' : 'metav'] = temp[key]
        }
    }    

   
}
console.log(util.inspect(obj["Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share"], false, null, true /* enable colors */))

fs.writeFileSync('results.json', JSON.stringify(obj), 'utf-8')
// console.log(obj);

// for(const familyName in modelFamilies){
//     // restore proper (lexicographical order wrt configuration values)
//     modelFamilies[familyName].sort( (x,y)=>{
//         let a=x.match(/\d+/g).map(j=>Number(j));
//         let b=y.match(/\d+/g).map(j=>Number(j));        
//         for(let ii=0; ii<a.length; ii++){
//             if(a[ii]<b[ii])return -1;
//             if(a[ii]>b[ii])return 1;
//         }
//         return 0;
//     })
    
//     if(familyName==='ssvr')continue;

//     for(const modelPath of modelFamilies[familyName]){
//         // console.log(modelPath);
//         let arr = computeAvgMeasurements(modelPath);
//         console.log(`${path.basename(modelPath)}\t${arr.join(';')}`);
//         // console.log(modelPath.match(/\d+/g));
//     }
// }

function computeAvgMeasurements(modelPath){
    // console.log(`Running computation for ${modelPath}`);
    let res;
    for(let irep = 0; irep < N_REPS; irep++){
        let curr = computeMeasurements(modelPath);

        if(typeof res ==='undefined'){
            res = curr;
        }else{
            for(let i=0;i<res.length;i++){
                res[i]+=curr[i];
            }
        }
    }
    for(let i=0;i<res.length;i++){
        res[i]=res[i]/N_REPS;
    }
    return res;
}

function computeMeasurements(modelPath){
    return execSync(
            `/usr/bin/time -f "%M %e %U %S" ./stv --file ${modelPath} 2>&1 >/dev/null`
            ,{shell: true}
        ).toString().split(' ').map(x=>Number(x));
}