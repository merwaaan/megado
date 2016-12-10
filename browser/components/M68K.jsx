import React from 'react';
import classnames from 'classnames';
import _ from 'lodash';

function hex(x, size = 2) {
    return ('0'.repeat(size) + x.toString(16).toUpperCase()).substr(-size, size)
}

class M68k extends React.Component {

    constructor(props) {
        super(props);

        // TODO make it stateless
        this.state = {
            d: _.times(8, _.random.bind(0xFF)),
            a: _.times(8, _.random.bind(0xFF)),
            pc: _.random(0xFF),
            ccr: _.random(0xFF)
        };

        this.state.a[7] = 5;
    }

    render() {

        const registerRows = _.range(8).map(i =>
            <tr key={i}>
                <td>D{i}</td>
                <td>{hex(this.state.d[i], 2)}</td>
                <td>A{i}</td>
                <td>{hex(this.state.a[i], 2)}</td>
            </tr>);

        const flags = _.map('CVZNX', (flag, bit) =>
            <span key={bit}
                  className={classnames('flag', {set: this.state.a[7] & (1 << bit)})}>
                {flag}
            </span>);

        return (
            <div className="m68k">
                <table>
                    <tbody>
                        {registerRows}
                        <tr>
                            <td>PC</td>
                            <td>{hex(this.state.pc, 2)}</td>
                            <td>SR</td>
                            <td>{hex(this.state.ccr, 2)}</td>
                        </tr>
                    </tbody>
                </table>
                {flags}
            </div>);
    }
}

M68k.propTypes = {

};

export default M68k;
