import React from 'react';
import {render} from 'react-dom';

import Debugger from 'components/Debugger';

import 'styles/main.scss';

class App extends React.Component {
    render () {
        return (
            <div>
            <p> Hello React!</p>
            <Debugger/>
            </div>
        );
    }
}

render(<App/>, document.querySelector('main'));
