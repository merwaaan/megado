import React from 'react';
import {render} from 'react-dom';

import Memory from 'components/Memory';
import M68k from 'components/M68k';
import Program from 'components/Program';

import 'styles/main.scss';

class App extends React.Component {
    render () {
        return (
            <div className="app">
                <Memory rowCount={20} />
                <Program rowCount={20} />
                <M68k />
            </div>
        );
    }
}

render(<App/>, document.querySelector('main'));
