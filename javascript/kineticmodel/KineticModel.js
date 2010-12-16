/*
  This file is part of the Ofi Labs X2 project.

  Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

var KineticModel = (function () {

    KineticModel = function (min, max, handler)
    {
        this.minimum = min;
        this.maximum = max;
        this.duration = 3300;
        this.position = 0;
        this.updateInterval = 15;

        this.state = {
            ticker: null,
            velocity: 0,
            deacceleration: 0,
            lastPosition: 0,
            timestamp: Date.now(),
            callback: handler,
            releaseT: null,
            thresholdV: 0,
            slowdown: false
        };
    };

    KineticModel.prototype = {

        clamped: function (pos) {
            return (pos > this.maximum) ? this.maximum :
                (pos < this.minimum) ? this.minimum: pos;
        },

        setPosition: function (pos) {
            var self = this;
            this.state.releaseT = null;
            this.position = this.clamped(pos);
            this.state.callback(this.position);
            if (this.state.ticker === null) {
                this.state.ticker = window.setInterval(function () {
                    var s = self.state,
                        elapsed = Date.now() - s.timestamp,
                        v = (self.position - s.lastPosition) * 1000 / elapsed;
                    if ((s.releaseT === null) && elapsed >= self.updateInterval) {
                        s.timestamp = Date.now();
                        if (v > 1 || v < -1) {
                            s.velocity = 0.2 * (s.velocity) + 0.8 * v;
                            s.lastPosition = self.position;
                        }
                    }
                }, this.updateInterval);
            }
        },

        resetSpeed: function () {
            var s = this.state;
            s.releaseT = null;
            s.velocity = 0;
            s.lastPosition = this.position;
            window.clearInterval(s.ticker);
            s.ticker = null;
        },

        release: function () {
            var s = this.state,
                self = this;
            window.clearInterval(s.ticker);
            s.ticker = null;
            if (s.velocity > 1 || s.velocity < -1) {
                s.releaseT = Date.now();
                s.timestamp = s.releaseT;
                s.deacceleration = 3200 * s.velocity / this.duration;
                s.thresholdV = Math.abs(0.33 * s.velocity);
                s.slowdown = false;
                window.clearInterval(s.ticker);
                s.ticker = window.setInterval(function () {
                    var s = self.state,
                        now = Date.now(),
                        elapsed = now - s.releaseT,
                        delta = (now - s.timestamp) / 1000,
                        vstep = s.deacceleration * delta,
                        oldv = s.velocity;
                    if (s.releaseT !== null) {
                        s.timestamp = now;
                        self.position = self.clamped(self.position + s.velocity * delta);
                        if (!s.slowdown && (Math.abs(s.velocity) < s.thresholdV)) {
                            s.slowdown = true;
                            s.deacceleration /= 8;
                            vstep = s.deacceleration * delta;
                        }
                        s.velocity = oldv - vstep;
                        if ((s.velocity < 0 && oldv > 0) || (s.velocity > 0 && oldv < 0) || (elapsed > self.duration)) {
                            self.resetSpeed();
                        }
                        s.callback(self.position);
                    }
                }, this.updateInterval);
            }
        }
    };

    return KineticModel;

}());
