import React from 'react';
import {render} from 'react-dom';

import Memory from 'components/Memory';
import M68k from 'components/M68k';
import Program from 'components/Program';

import 'styles/main.scss';

function saveRomInBrowser(data) {
    console.log('Saving to browser storage...');

    const string = new TextDecoder('utf8').decode(data);
    localStorage.setItem('rom', string);
    console.log(data, string)
    console.log(`File saved (${data.length} bytes)`);
}

function loadRomFromBrowser() {
    console.log('Checking browser storage...');

    const rom = localStorage.getItem('rom');
    if (rom) {
        const data = new TextEncoder('utf8').encode(rom);
        console.log(`File found (${data.length} bytes)`);
        return data;
    }
    else
        console.log('No file found');
}

class App extends React.Component {

    constructor() {
        super();

        this.state = {
            genesisPtr: null,
            memoryPtr: null,
            m68kPtr: null,
        }
    }

    render () {
        const d = Module.HEAP8[this.state.m68kPtr];

        return (
            <div className="app">
                <input type="file" onChange={this.handleFile.bind(this)}/>
                <div className="debugger">
                    <Memory start={this.state.memoryPtr}
                            rowCount={20} />
                    <Program start={this.state.memoryPtr}
                             pc={this.state.pc}
                             rowCount={10} />
                    <M68k />
                </div>
            </div>
        );
    }

    componentWillMount() {
        this.state.genesisPtr = Module.ccall('genesis_make', 'number');
        console.log(`Genesis @${this.state.genesisPtr}`);

        this.state.memoryPtr = Module.ccall('genesis_memory', 'number', ['number'], [this.state.genesisPtr]);
        console.log(`Memory @${this.state.memoryPtr}`);

        this.state.m68kPtr = Module.ccall('genesis_m68k', 'number', ['number'], [this.state.genesisPtr]);
        console.log(`M68000 @${this.state.m68kPtr}`);

        //const data = loadRomFromBrowser();
        //if (data)
        //    loadRom(data);
    }

    handleFile(event) {
        const reader = new FileReader();
        const file = event.target.files[0];

        reader.onload = event => {
            const data = new Uint8Array(event.target.result);

            //saveRomInBrowser(data);
            this.loadRom(data);
        };

        console.log(`Loading "${file.name}"...`);
        reader.readAsArrayBuffer(file);
    }

    loadRom(data) {
        Module.HEAP8.set(data, this.state.memoryPtr);
        this.forceUpdate();
    }
}

render(<App/>, document.querySelector('main'));
