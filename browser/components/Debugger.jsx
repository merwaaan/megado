import React from 'react';
import classnames from 'classnames';
import _ from 'lodash';

function hex(x, size = 2) {
    return ('0'.repeat(size) + x.toString(16).toUpperCase()).substr(-size, size)
}

const memory = _.times(0x10000, _.random.bind(0x100)); // TODO use external data

class Debugger extends React.Component {

    constructor(props) {
        super(props);

        this.state = {
            offset: 0,
            searchedAddress: 0
        };
    }

    render() {

        const header =
            <tr>
                <td>
                    <input value={this.state.searchedAddress.toString(16)}
                           onChange={this.search.bind(this)}/>
                </td>
                {_.range(16).map(i => <td key={i}>{hex(i, 2)}</td>)}
            </tr>;

        const rows = _.range(this.state.offset, this.state.offset + this.props.rowCount * 16, 16).map(offset =>
            <tr key={offset}>

                { console.log(offset)/* Row offset */ }
                <td>{hex(offset, 8)}</td>

                { /* Row values */ }
                {_.range(offset, offset + 16).map(addr =>
                    <td key={addr}
                        className={classnames({'target': addr === this.state.searchedAddress})}>
                        {hex(memory[addr], 2)}
                        </td>)}
            </tr>);

        return (<div className="debugger">
            <table>
                <tbody>
                    {header}
                    {rows}
                </tbody>
            </table>
            <section className="controls">
                <i className="fa fa-fast-backward fa-rotate-90" onClick={this.move.bind(this, Number.NEGATIVE_INFINITY)}></i>
                <i className="fa fa-step-backward fa-rotate-90" onClick={this.move.bind(this, -16)}></i>
                <i className="fa fa-step-forward fa-rotate-90" onClick={this.move.bind(this, +16)}></i>
                <i className="fa fa-fast-forward fa-rotate-90" onClick={this.move.bind(this, Number.POSITIVE_INFINITY)}></i>
            </section>
        </div>);
    }

    move(offset) {
        this.state.offset = _.clamp(this.state.offset + offset, 0, 0x10000); // TODO use real size
        this.setState(this.state);
    }

    search (event) {

        // Filter non-hexadecimal characters
        let address = event.target.value.replace(/[^A-Fa-f0-9]/g, '');
        address = address.length > 0 ? parseInt(address, 16) : 0;

        this.state.searchedAddress = address;
        this.state.offset = Math.floor(address / 16) * 16;
        this.setState(this.state);
    }
}

Debugger.propTypes = {
    rowCount: React.PropTypes.number.isRequired
};

export default Debugger;
